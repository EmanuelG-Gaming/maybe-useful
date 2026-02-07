#ifndef ARENA_H_
#define ARENA_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////
// Macros
// The majority of this header file is macros.
// THough, a lot of them can be added to your projects!
///////////////////////////////////////////////////////////////////////////////


// Add these macros if you want to check for OS/compiler

///////////////////////////////////////////////////////////////////////////////
// Operating systems
///////////////////////////////////////////////////////////////////////////////

// Windows
#if !defined(HAS_WINDOWS)
#if defined(_WIN32)
#define HAS_WINDOWS 1
#else
#define HAS_WINDOWS 0
#endif
#endif

// UNIX (it's more than just MacOS)
#if !defined(HAS_UNIX)
#if !defined(_WIN32) && ((defined(__unix__) || defined(__unix)) || \
                        (defined(__APPLE__) && defined(__MACH__)))
#define HAS_UNIX 1
#else
#define HAS_UNIX 0
#endif

// Linux
#if !defined(HAS_LINUX)
#if defined(__linux__)
#define HAS_LINUX 1
#else
#define HAS_LINUX 0
#endif  
#endif

///////////////////////////////////////////////////////////////////////////////
// Compiler checks
// Note that Clang supports both GCC and MSVC, so we need to separate them.
///////////////////////////////////////////////////////////////////////////////

// MSVC
#if !defined(HAS_MSVC)
#if defined(_MSC_VER) && !defined(__clang__)
#define HAS_MSVC 1
#else
#define HAS_MSVC 0
#endif
#endif

// Clang
#if !defined(HAS_CLANG)
#if defined(__clang__)
#define HAS_CLANG 1
#else
#define HAS_CLANG 0
#endif
#endif

#endif

// GCC
#if !defined(HAS_GCC)
#if defined(__GNUC__) && !defined(__clang__)
#define HAS_GCC 1
#else
#define HAS_GCC 0
#endif
#endif

// TCC (Tiny C Compiler)


///////////////////////////////////////////////////////////////////////////////
// Alignment issues
///////////////////////////////////////////////////////////////////////////////

#if HAS_MSVC
#define ALIGN_OF(type) __alignof(type)
#elif HAS_CLANG
#define ALIGN_OF(type) __alignof(type)
#elif HAS_GCC | HAS_TCC
#define ALIGN_OF(type) __alignof__(type)
#else
#error "Unsupported compiler!"
#endif

///////////////////////////////////////////////////////////////////////////////
// Thread local variables
///////////////////////////////////////////////////////////////////////////////

#if HAS_MSVC
#define THREAD_LOCAL __declspec(thread)
#elif HAS_CLANG || HAS_GCC || HAS_TCC
#define THREAD_LOCAL __thread
#else
#error "Unsupported compiler!"
#endif



///////////////////////////////////////////////////////////////////////////////
// More macros
///////////////////////////////////////////////////////////////////////////////

#define KB(n) (((uint64_t)(n)) << 10)
#define MB(n) (((uint64_t)(n)) << 20)
#define GB(n) (((uint64_t)(n)) << 30)
#define TB(n) (((uint64_t)(n)) << 40)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// A compile-time assert that shows up as an array having a negative size,
// generating an error.
// This is used in C code or in old C++ code
// that doesn't have static_assert() (added in C++11).
#define STATIC_ASSERT(condition, msg) \
    typedef char static_assertion__##msg[(condition) ? 1 : -1]

#define ARRAY_COUNT(array) (int)((sizeof(array) / sizeof(array[0])))


///////////////////////////////////////////////////////////////////////////////
// Assertion/error checking macros
///////////////////////////////////////////////////////////////////////////////

#ifndef FAIL_MESSAGE
#define FAIL_MESSAGE(...) \
    do { \
        fprintf(stderr, "FATAL ERROR: " __VA_ARGS__); \
        fprintf(stderr, "\n At file %s:%d in %s()\n", __FILE__, __LINE__, __func__); \
        abort(); \
    } while(0)
#endif // FAIL_MESSAGE

#ifndef ASSERT
#define ASSERT(expr) do { if (!(expr)) FAIL_MESSAGE("Assertion failed: %s\n", #expr); } while(0)
#endif // ASSERT

///////////////////////////////////////////////////////////////////////////////
// OS-dependent things
///////////////////////////////////////////////////////////////////////////////

typedef struct OS_System_Info {
    uint32_t logical_processor_count;
    uint64_t page_size;
    uint64_t large_page_size;
    uint64_t allocation_granularity;
} OS_System_Info;

void os_get_system_info(OS_System_Info **out);

void* os_memory_reserve(size_t size);
void os_memory_release(void* ptr, size_t size);

void os_memory_commit(void* ptr, size_t size);
void os_memory_decommit(void* ptr, size_t size);

///////////////////////////////////////////////////////////////////////////////
// Arena definition
///////////////////////////////////////////////////////////////////////////////

// 128 bytes is the maximum for an arena struct
#define ARENA_HEADER_MAX_SIZE 128
#define ARENA_USE_FREE_LIST 1

// Simple Malloc
#define ARENA_USE_MALLOC 0
#define ARENA_DEFAULT_RESERVE_SIZE MB(1)
#define ARENA_DEFAULT_COMMIT_SIZE KB(64)


typedef enum {
    ARENA_FLAG_NONE = 0,
    ARENA_FLAG_NO_CHAIN = 1 << 0,
} Arena_Flags;
typedef uint32_t Arena_Flag_t;


typedef struct Arena {
    struct Arena *prev;
    struct Arena *current;

    Arena_Flag_t flags;
    uint64_t committed_size;
    uint64_t reserved_size;

    // Base position + position = absolute position in memory
    uint64_t base_position;
    uint64_t position;

    uint64_t commit_pos;
    uint64_t reserved_pos;

    // For free list
    uint64_t free_size;
    struct Arena *free_last;
} Arena;
STATIC_ASSERT(sizeof(Arena) <= ARENA_HEADER_MAX_SIZE,
        expected_arena_header_to_be_smaller_or_equal_to_128_bytes);

typedef struct Arena_Config {
    uint64_t reserve_size;
    uint64_t commit_size;

    Arena_Flag_t flags;
} Arena_Config;

// Like in MagicalBat video
// This simply holds a reference to the arena that's being owned in memory.
typedef struct Arena_Temp {
    struct Arena *arena;
    uint64_t position;
} Arena_Temp;

void arena_system_init(void);
void arena_system_deinit(void);

// Spawn an arena with custom config
Arena* arena_alloc_from_config(Arena_Config* config);
// Spawn an arena with default config
Arena* arena_alloc(void);

// Release the arena and all of its blocks
void arena_release(Arena* self);


///////////////////////////////////////////////////////////////////////////////
// Debugging
///////////////////////////////////////////////////////////////////////////////
void arena_print(Arena* self);

///////////////////////////////////////////////////////////////////////////////
// Adding things to arena
///////////////////////////////////////////////////////////////////////////////

void* arena_push(Arena* self, uint64_t size, uint64_t align);

// A C string has a null terminator, from which its length can be determined
char* arena_push_cstr(Arena* self, const char* str);
// Push formatted string to arena
char* arena_push_cstr_fmt(Arena* self, const char* fmt, ...);
// Push formatted string to arena, with variadic arguments
char* arena_push_cstr_fmt_va(Arena* self, const char* fmt, va_list args);

uint64_t arena_position(Arena* self);

// Pop/free all the memory in the arena, up to the given position
void arena_pop_to(Arena* self, uint64_t position);
// Resets/clears the arena of all its elements
void arena_clear(Arena* self);
// Pop n bytes from an arena, like a stack
void arena_pop(Arena* self, uint64_t n);

void* arena_get_base_addr(Arena* self);


///////////////////////////////////////////////////////////////////////////////
// Temporary arenas
///////////////////////////////////////////////////////////////////////////////

Arena_Temp arena_temp_begin(Arena* arena);
void arena_temp_end(Arena_Temp temp);

//void arena_scratch_alloc(void);
//void arena_scratch_release(void);

// Starts a scratch arena for temporary allocations
Arena_Temp arena_scratch_begin(Arena** conflicts, int conflict_count);
// Ends a scratch arena
void arena_scratch_end(Arena_Temp scratch);

///////////////////////////////////////////////////////////////////////////////
// Helper macros for appending things
///////////////////////////////////////////////////////////////////////////////

// Does NOT set to 0
#define arena_push_array_aligned_no_zero(arena, T, count, align) \
    (T *) arena_push((arena), (count) * sizeof(T), (align))

#define arena_push_array_aligned(arena, T, count, align) \
    (T *) memset(arena_push_array_aligned_no_zero((arena), T, (count), (align)), 0, (count) * sizeof(T))

// 8 bytes is the size of the alignment boundaries,
// because, for maximum performance on 64-bit systems,
// it helps with fetching only one instruction for the CPU,
// rather than 2
#define arena_push_array_no_zero(arena, T, count) \
    arena_push_array_aligned_no_zero((arena), T, (count), MAX(8, ALIGN_OF(T)))

#define arena_push_array(arena, T, count) \
    arena_push_array_aligned((arena), T, (count), MAX(8, ALIGN_OF(T)))

#define arena_push_struct_no_zero(arena, T) \
    arena_push_array_no_zero((arena), T, sizeof(T), MAX(8, ALIGN_OF(T)))

#define arena_push_struct(arena, T) \
    arena_push_array((arena), T, sizeof(T))

// Could be turned into a header-only library

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ARENA_H_
