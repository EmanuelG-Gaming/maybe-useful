#include "arena.h"
#include <endian.h>
#include <stdint.h>
#include <string.h>

// Obligatory includes
#if HAS_LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#elif HAS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#error "OS not supported. It must be Windows or Linux, not MacOS or UNIX. Sorry!"
#endif

#include <stdio.h>
#include <stdlib.h>

#define ALIGN_UP_POW2(n, p) (((n) * ((p) - 1)) & (~((p) - 1)))

#define CLAMP_TOP(a, max) MIN(a, max)
#define CLAMP_BOTTOM(a, min) MAX(a, min)


///////////////////////////////////////////////////////////////////////////////
// OS-dependent things
///////////////////////////////////////////////////////////////////////////////

void os_get_system_info(OS_System_Info **out)
{
    static OS_System_Info os_info = { 0 };
    static int is_cached = 0;

    if (!is_cached) {
#if HAS_ANDROID
        os_info.logical_processor_count = sysconf(_SC_NPROCESSORS_ONLN);
        os_info.page_size = (uint64_t) getpagesize();
        os_info.large_page_size = MB(2);
        os_info.allocation_granularity = os_info.page_size;
#elif HAS_LINUX
        os_info.logical_processor_count = (uint32_t) get_nprocs();
        os_info.page_size = (uint64_t) getpagesize();
        os_info.large_page_size = MB(2);
        os_info.allocation_granularity = os_info.page_size;
#elif HAS_WINDOWS
        SYSTEM_INFO sys_info = { 0 };
        GetSystemInfo(&sys_info);
        os_info.logical_processor_count = sys_info.dwNumberOfProcessors;
        os_info.page_size = sys_info.dwPageSize;
        os_info.large_page_size = GetLargePageMinimum();
        os_info,allocation_granularity = sys_info.dwAllocationGranularity;
#else
#error "OS not supported"
#endif
        is_cached = 1;
    }
    *out = &os_info;
}

void* os_memory_reserve(size_t size)
{
#if HAS_LINUX
    void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        result = 0;
    }
    return result;
#elif HAS_WINDOWS
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
#else
#error "Unsupported OS for reserving memory"
#endif
}

void os_memory_release(void *ptr, size_t size)
{
#if HAS_LINUX
    munmap(ptr, size);
#elif HAS_WINDOWS
    (void) size;
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
#error "Unsupported OS for releasing memory"
#endif
}

void os_memory_commit(void *ptr, size_t size)
{
#if HAS_LINUX
    int res = mprotect(ptr, size, PROT_READ | PROT_WRITE);
    if (res == 1) {
        FAIL_MESSAGE("mprotect failed to commit memory: %s", strerror(errno));
    }
#elif HAS_WINDOWS
    VirtualAlloc(ptr, size, MEM_COMMIT, MEM_READWRITE);
#else
#error "Unsupported OS for committing memory"
#endif
}

void os_memory_decommit(void *ptr, size_t size)
{
#if HAS_LINUX
    madvise(ptr, size, MADV_DONTNEED);
    mprotect(ptr, size, PROT_NONE);
#elif HAS_WINDOWS
    VirtualFree(ptr, size, MEM_DECOMMIT);
#else
#error "Unsupported OS for decommitting memory"
#endif
}


///////////////////////////////////////////////////////////////////////////////
// Arena implementation
///////////////////////////////////////////////////////////////////////////////

static void arena_scratch_alloc(void);
static void arena_scratch_release(void);

void arena_system_init(void)
{
    arena_scratch_alloc();
}

void arena_system_deinit(void)
{
    arena_scratch_release();
}

Arena* arena_alloc_from_config(Arena_Config* config)
{
    uint64_t reserve_size = config->reserve_size + ARENA_HEADER_MAX_SIZE;
    uint64_t commit_size = config->commit_size;

    OS_System_Info* system_info;
    os_get_system_info(&system_info);

    reserve_size = ALIGN_UP_POW2(reserve_size, system_info->page_size);
    commit_size = ALIGN_UP_POW2(commit_size, system_info->page_size);

    void *base = os_memory_reserve(reserve_size);
    os_memory_commit(base, commit_size);

    if (!base) {
        FAIL_MESSAGE("Failed to reserve memory for arena");
    }

    Arena *arena = (Arena *) base;
    arena->current = arena; // Pointer to itself
    arena->committed_size = commit_size;
    arena->reserved_size = reserve_size;
    arena->base_position = 0;
    arena->position = ARENA_HEADER_MAX_SIZE;
    arena->commit_pos = commit_size;
    arena->reserved_pos = reserve_size;

    arena->free_size = 0;
    arena->free_last = NULL;

    // Address sanitizer support?

    return arena;
}

Arena* arena_alloc(void)
{
    Arena_Config config = {
        .reserve_size = ARENA_DEFAULT_RESERVE_SIZE,
        .commit_size = ARENA_DEFAULT_COMMIT_SIZE,
        
        .flags = ARENA_FLAG_NONE,
    };
    return arena_alloc_from_config(&config);
}

void arena_release(Arena* self)
{
    for (Arena *n = self->current, *prev = NULL; n != NULL; n = prev) {
        prev = n->prev;
        os_memory_release(n, n->reserved_pos);
    }
}

void* arena_push(Arena *self, uint64_t size, uint64_t align)
{
    // Check if this arena exists
    ASSERT(self);

    if (size == 0) {
        return NULL;
    }

    Arena *current = self->current;
    // Boundaries of the allocated region
    uint64_t position_prev = ALIGN_UP_POW2(current->position, align);
    uint64_t position_post = position_prev + size;

    // Chain arenas if there isn't enough space
    if (current->reserved_pos < position_post && !(self->flags & ARENA_FLAG_NO_CHAIN)) {
        Arena *new_block = NULL;

#ifdef ARENA_USE_FREE_LIST
        // Try to find a free block that is big enough to fit the allocation
        Arena *prev_block;

        for (new_block = self->free_last, prev_block = NULL; new_block != NULL; prev_block = new_block, new_block = new_block->prev) {
            if (new_block->reserved_pos >= ALIGN_UP_POW2(size, align)) {
                if (!prev_block) {
                    prev_block->prev = new_block->prev;
                } else {
                    self->free_last = new_block->prev;
                }

                self->free_size -= new_block->reserved_size;
                // ASAN unpoison memory region
                break;
            }
        }
#endif
        // Make a new block if there isn't any
        if (!new_block) {
            uint64_t reserved_size = current->reserved_size;
            uint64_t commit_size = current->committed_size;
            
            if (size + ARENA_HEADER_MAX_SIZE > reserved_size) {
                reserved_size = ALIGN_UP_POW2(size + ARENA_HEADER_MAX_SIZE, align);
                commit_size = ALIGN_UP_POW2(size + ARENA_HEADER_MAX_SIZE, align);

                Arena_Config config = {
                    .reserve_size = reserved_size,
                    .commit_size = commit_size,
                    .flags = ARENA_FLAG_NONE,
                };
                new_block = arena_alloc_from_config(&config);
            }
        }

        new_block->base_position = current->base_position + current->reserved_pos;
        new_block->prev = self->current;
        self->current = new_block;
        current = new_block;
        position_prev = ALIGN_UP_POW2(current->position, align);
        position_post = position_prev + size;
        ASSERT(position_post <= current->reserved_pos);
    }
    
    // Commit new pages, if needed
    if (current->commit_pos < position_post) {
        uint64_t commit_post_aligned = position_post + current->committed_size - 1;
        commit_post_aligned -= commit_post_aligned % current->committed_size;
        uint64_t commit_post_clamped = CLAMP_TOP(commit_post_aligned, current->reserved_pos);
        uint64_t commit_size = commit_post_clamped - current->commit_pos;
        uint8_t *commit_ptr = (uint8_t *) current + current->commit_pos;

        os_memory_commit(commit_ptr, commit_size);
    }

    // Push unto current block
    void *result = NULL;
    if (current->commit_pos >= position_post) {
        result = ((uint8_t *) current) + position_prev;
        current->position = position_post;
        // Unpoison memory region
    }

    // Panic on failure
    if (!result) {
        FAIL_MESSAGE("Arena allocation failed!");
    }

    return result;
}


char* arena_push_cstr(Arena* self, const char* str)
{
    ASSERT(self);

    size_t length = strlen(str);
    char *dest = arena_push_array(self, char, length + 1);
    strncpy(dest, str, length + 1); // Also add the null terminator
    return dest;
}


char* arena_push_cstr_fmt(Arena* self, const char* fmt, ...)
{
    ASSERT(self);

    va_list args;
    va_start(args, fmt);
    char *result = arena_push_cstr_fmt_va(self, fmt, args);
    va_end(args);
    return result;
}

char* arena_push_cstr_fmt_va(Arena* self, const char* fmt, va_list args)
{
    ASSERT(self);
    char temp[1024];
    int length = snprintf(temp, sizeof(temp), fmt, args);
    if (length < 0) {
        FAIL_MESSAGE("Failed to format string");
    }

    char *dest = arena_push_array(self, char, (uint64_t) length + 1);
    strncpy(dest, temp, length + 1);
    return dest;
}


uint64_t arena_position(Arena *self)
{
    ASSERT(self);
    Arena *current = self->current;
    return current->base_position + current->position;
}

void arena_pop_to(Arena *self, uint64_t position)
{
    uint64_t big_position = CLAMP_BOTTOM(ARENA_HEADER_MAX_SIZE, position);
    Arena *current = self->current;
#ifdef ARENA_USE_FREE_LIST
    for (Arena *prev = NULL; current->base_position >= big_position; current = prev) {
        prev = current->prev;

        current->position = ARENA_HEADER_MAX_SIZE;
        self->free_size += current->reserved_size;
        current->prev = self->free_last;

        // Poison memory region
    }
#else
    for (Arena *prev = NULL; current->base_position >= big_position; current = prev) {
        prev = current->prev;
        os_memory_release(current, current->reserved);
    }
#endif
    self->current = current;
    uint64_t new_position = big_position - current->base_position;

    ASSERT(new_position <= current->position);
    // Poison memory region
    current->position = new_position;
}

void arena_clear(Arena *self)
{
    // Cursor shall go to position 0
    arena_pop_to(self, 0);
}

void arena_pop(Arena *self, uint64_t n)
{
    uint64_t position_old = arena_position(self);
    uint64_t position_new = position_old;

    // Subtract position
    if (n < position_old) {
        position_new = position_old - n;
    }
    arena_pop_to(self, position_new);
}


void* arena_get_base(Arena* self)
{
    return (uint8_t *) self + ARENA_HEADER_MAX_SIZE;
}

Arena_Temp arena_temp_begin(Arena *arena)
{
    uint64_t position = arena_position(arena);
    Arena_Temp tmp = {
        .arena = arena,
        .position = position,
    };
    return tmp;
}

void arena_temp_end(Arena_Temp temp)
{
    arena_pop_to(temp.arena, temp.position);
}


///////////////////////////////////////////////////////////////////////////////
// Debugging
///////////////////////////////////////////////////////////////////////////////
void arena_print(Arena* self)
{
    (void) self;
    /*
    ASSERT(self);
    printf("Arena to: %zu, contents: ", self->base_position);
    fwrite(self->base_position, 1, self->committed_size, stdout);
    printf("\n");
    */
}

///////////////////////////////////////////////////////////////////////////////
// Scratch arenas
///////////////////////////////////////////////////////////////////////////////

static THREAD_LOCAL Arena *_scratch_arenas[2];

static void set_scratch_arena(int index, Arena* arena)
{
    // Bounds checking
    ASSERT(index >= 0 && index < (int) ARRAY_COUNT(_scratch_arenas));
    _scratch_arenas[index] = arena;
}

static Arena* scratch_arena(int index)
{
    ASSERT(index >= 0 && index < (int) ARRAY_COUNT(_scratch_arenas));
    return _scratch_arenas[index];
}

void arena_scratch_alloc(void)
{
    for (int i = 0; i < ARRAY_COUNT(_scratch_arenas); ++i) {
        if (!scratch_arena(i)) {
            set_scratch_arena(i, arena_alloc());
        }
    }
}

void arena_scratch_release(void)
{
    for (int i = 0; i < ARRAY_COUNT(_scratch_arenas); ++i) {
        if (scratch_arena(i)) {
            arena_release(scratch_arena(i));
            set_scratch_arena(i, NULL);
        }
    }
}

Arena_Temp arena_scratch_begin(Arena **conflicts, int conflict_count)
{
    Arena_Temp scratch = { 0 };
    for (int i = 0; i < ARRAY_COUNT(_scratch_arenas); ++i) {
        int is_conflicting = 0;
        for (int j = 0; j < conflict_count; ++j) {
            Arena *conflict = conflicts[j];
            if (scratch_arena(i) == conflict) {
                is_conflicting = 1;
                break;
            }
        }

        if (is_conflicting == 0) {
            scratch.arena = scratch_arena(i);
            scratch.position = scratch.arena->position;
            break;
        }
    }

    return scratch;
}

void arena_scratch_end(Arena_Temp scratch)
{
    arena_temp_end(scratch);
}
