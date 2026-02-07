#include "gamedev.h"
#include "arena.h"
#include "object_pool.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

const char *g_scene_name;

static bool _should_switch_scene;
static char _new_scene_name[32];

///////////////////////////////////////////////////////////////////////////////
// Timing system (needs while loop to poll)
///////////////////////////////////////////////////////////////////////////////

void time_init(Timing_System *self)
{
    if (!self) {
        return;
    }

    self->pool = (Timed_Task *) malloc(sizeof(Timed_Task) * 300);
    if (!self->pool) {
        FAIL_MESSAGE("Failed to allocate object pool!");
    }

    //self->head = NULL;
    objpool_init(self, 300);
}


Timed_Task* time_schedule(Timing_System *self, float delay, Task_Consumer cons)
{
    Timed_Task *new_task;
    objpool_add(self, new_task);

    float current_time = time_time();

    new_task->cons = cons;
    new_task->snapshot = current_time + delay;
    new_task->delay = delay;

    return new_task;
}

void time_run_due_tasks(Timing_System *self)
{
    float current_time = time_time();

    size_t count = self->count;
    for (int i = 0; i < self->count; ++i) {
        Timed_Task *current = &self->pool[i];
        if (current->snapshot <= current_time) {
            current->cons();

            objpool_remove(self, current);
        }
    }
    printf("%zu\n", count);
}



void time_repeat_delayed(Timing_System *self,
    float initial_delay, float interval, unsigned int count, Task_Consumer cons)
{
    for (int i = 0; i < count; ++i) {
        time_schedule(self, initial_delay + interval * i, cons);
    }
}

float time_time(void)
{
    return time(NULL);
}


///////////////////////////////////////////////////////////////////////////////
// Event system
///////////////////////////////////////////////////////////////////////////////

void events_notify_update(Event_Callbacks *subscribers, Event_t *ev)
{
    for (unsigned int i = 0; i < subscribers->count; ++i) {
        Event_Callback *callback = &subscribers->data[i];
        callback->update(callback->arg);
    }
}

void events_notify_input(Event_Callbacks *subscribers, Event_t *ev)
{
    for (unsigned int i = 0; i < subscribers->count; ++i) {
        Event_Callback *callback = &subscribers->data[i];
        // Only call specific events this way
        if (callback->type_mask & ev->type) {
            callback->input(callback->arg, ev);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Scene system
///////////////////////////////////////////////////////////////////////////////

void scene_switch(const char *to)
{
    strncpy(_new_scene_name, to, 32);

    _should_switch_scene = true;
    /*
    if (_f_load_start) {
        _f_load_start(loadArg);
    }
    */
}

// Deferred action
static void scene_do_switch(void)
{
Event_t ev;
    if (!_should_switch_scene) {
        return;
    }

    if (g_scene_name) {
        // End the previous scene
        ev.type = EVENT_END_SCENE;
        //events_notify_input(&inputHandlers, &ev, false);
    }
    // Something

    // Start the new scene
    g_scene_name = _new_scene_name;
    ev.type = EVENT_START_SCENE;
    //events_notify_input(&inputHandlers, &ev, false);

    _should_switch_scene = false;
}

///////////////////////////////////////////////////////////////////////////////
// Entity component system (ECS)
///////////////////////////////////////////////////////////////////////////////

#define DENSE_PAGE_SIZE 1024
#define DENSE_PAGE_LIST_SIZE (MAX_ENTITIES / DENSE_PAGE_SIZE)
#define SPARSE_NONE ((Idx_t)-1)

typedef struct Component_List {
    bool initialized;

    // Flags
    bool no_purge;
    bool deleted_component;
    bool keep_ordering;

    unsigned int component_size;
    unsigned int count;
    unsigned int n_dense_pages;

    // Create/delete notifier
    //void *arg;

    Idx_t *sparse;
    void *dense[DENSE_PAGE_LIST_SIZE];
} Component_List;

Component_List component_lists[MAX_COMPONENT_LISTS];

uint32_t first_free_entity;
Entity_t entity_list[MAX_ENTITIES];
uint64_t entity_component_lists[MAX_ENTITIES * 2];


void ecs_cl_init_sz(int id, unsigned int element_size)
{
    Component_List *cl = &component_lists[id];
    
    // Set to 0
    memset(cl, 0, sizeof(*cl));

    cl->initialized = true;
 cl->component_size = element_size;

    cl->sparse = (Idx_t *) malloc(sizeof(Idx_t) * MAX_ENTITIES);
    if (!cl->sparse) {
        FAIL_MESSAGE("Couldn't allocate memory for sparse component list");
    }

    for (unsigned int i = 0; i < MAX_ENTITIES; ++i) {
        cl->sparse[i] = SPARSE_NONE;
    }
    printf("New component list: (%d)\n", id);
}

void ecs_cl_deinit(int id)
{
    // Might have to use an internal arena for a component list.

    Component_List *cl = &component_lists[id];
    cl->initialized = false;

    for (unsigned int i = 0; i < cl->n_dense_pages; ++i) {
        free(cl->dense[i]);
    }
    free(cl->sparse);
}

static inline Entity_t* ecs_dense_at(Component_List *cl, Idx_t idx)
{
    return (Entity_t *) (
        (char *) (cl->dense[idx / DENSE_PAGE_SIZE]) +
        cl->component_size * (idx % DENSE_PAGE_SIZE)
    );
}


void* ecs_get_component_nullable(int id, Entity_t entity)
{
    Component_List *cl = &component_lists[id];
    Idx_t idx = cl->sparse[entity >> ENTITY_ID_SHIFT];

    if (idx != SPARSE_NONE) {
        // If there is something in the sparse check,
        // that means there is something dense.
        Entity_t *e = ecs_dense_at(cl, idx);
        if (entity == *e) {
            return e;
        }
    }
    return NULL;
}
void* ecs_get_component(int id, Entity_t entity)
{
    void *ret = ecs_get_component_nullable(id, entity);
    if (!ret) {
        FAIL_MESSAGE("Couldn't get the component at id (%d) from entity", id);
    }
    return ret;
}

void* ecs_new_component(int id, Entity_t entity)
{
    Component_List *cl = &component_lists[id];
    if (!cl->initialized) {
        FAIL_MESSAGE("Couldn't make a new component because the component list at index (%d) was not initialized!", id);
    }

    Entity_t sparse_idx = entity >> ENTITY_ID_SHIFT;
    Idx_t idx;
    if (cl->sparse[sparse_idx] != SPARSE_NONE) {
        // Overwrite component if it already exists
        idx = cl->sparse[sparse_idx];
        printf("Overwriting component %x in list %d\n", entity, id);
    } else {
        // We need to calculate the push index and place dense
        // components at the corresponding location
        idx = cl->count;
        if (idx % DENSE_PAGE_SIZE == 0) {
            size_t sz = (size_t) DENSE_PAGE_SIZE * cl->component_size;
            char *dense = (char *) malloc(sz + 4);
            if (!dense) {
                FAIL_MESSAGE("Couldn't allocate dense memory block!");
            }

            // I love pointer arithmetic
            *(Entity_t *)(dense + sz) = ((idx / DENSE_PAGE_SIZE) + 1) << ENTITY_ID_SHIFT;
            cl->dense[idx / DENSE_PAGE_SIZE] = dense;
            cl->n_dense_pages++;
        }
        cl->count++;
    }
    
    void *result = ecs_dense_at(cl, idx);
    memset(result, 0, cl->component_size);
    *(Entity_t *) result = entity;
    
    entity_component_lists[sparse_idx*2 + id / 64] |= (1ULL << id % 64);

    // We've now created the component

    return result;
}

static void ecs_unreg_entity(Entity_t entity);
void ecs_remove_component(int id, Entity_t entity)
{
    Component_List *cl = &component_lists[id];
    Entity_t sparse_idx = entity >> ENTITY_ID_SHIFT;
    Idx_t idx = cl->sparse[sparse_idx];

    if (idx == SPARSE_NONE) {
        return;
    }
    cl->sparse[sparse_idx] = SPARSE_NONE;

    Entity_t *dst = ecs_dense_at(cl, idx);
    if (entity != *dst) {
        // Deleting an older version
        return;
    }

    // Deletion

    if (cl->keep_ordering) {
        *dst = 0;
    } else {
        cl->count -= 1;
        if (idx != cl->count) {
            // Do swap
            Entity_t *src = ecs_dense_at(cl, cl->count);
            memcpy(dst, src, cl->component_size);
            Entity_t swapped = *dst;
            cl->sparse[swapped >> ENTITY_ID_SHIFT] = idx;
            *src = 0;
        } else {
            *dst = 0;
        }

        if (cl->count % DENSE_PAGE_SIZE == 0) {
            uint32_t i = cl->count / DENSE_PAGE_SIZE;
            free(cl->dense[i]);
            cl->dense[i] = NULL;
            cl->n_dense_pages--;
        }
        cl->deleted_component = true;
    }

    entity_component_lists[sparse_idx*2 + id/64] &= ~(1ULL << id % 64);
    if (!entity_component_lists[sparse_idx*2] &&
        !entity_component_lists[sparse_idx*2 + 1]) {
        // Remove this entity if it doesn't have any more components
        ecs_unreg_entity(entity);
    }
}

void ecs_cl_ordered_clean(int id)
{
    Component_List *cl = &component_lists[id];
    
    int src_idx, dst_idx;
    src_idx = dst_idx = 0;
    
    for (unsigned int i = 0; i < cl->count; ++i) {
        Entity_t *src = ecs_dense_at(cl, src_idx);
        if (*src) {
            if (src_idx != dst_idx) {
                Entity_t *dst = ecs_dense_at(cl, dst_idx);
                memcpy(dst, src, cl->component_size);
                *src = 0;
                cl->sparse[*dst >> ENTITY_ID_SHIFT] = dst_idx;
            }
            dst_idx++;
        }
    }

    cl->count = dst_idx;
    int start = (cl->count / DENSE_PAGE_SIZE);
    if (cl->count % DENSE_PAGE_SIZE) {
        start++;
    }

    for (int i = start; i < cl->n_dense_pages; ++i) {
        if (cl->dense[i]) {
            free(cl->dense[i]);
            cl->dense[i] = NULL;
        }
    }

    cl->n_dense_pages = start;
}



Idx_t ecs_cl_count(int id)
{
    Component_List *cl = &component_lists[id];
    return cl->count;
}

void* ecs_cl_at(int id, Idx_t index)
{
    Component_List *cl = &component_lists[id];
    return ecs_dense_at(cl, index);
}

void* ecs_cl_begin(int id)
{
    Component_List *cl = &component_lists[id];
    cl->deleted_component = false;
    return cl->dense[0];
}

void* ecs_cl_next(int id, void *iter)
{
    Component_List *cl = &component_lists[id];

    Entity_t *next = 0;
    if (cl->deleted_component) {
        cl->deleted_component = false;
        if (!cl->count) {
            return NULL;
        }
        next = (Entity_t *) iter;
    } else {
        next = (Entity_t *)((char *) iter + cl->component_size);
    }

    if (!(*next & ENTITY_VERSION_MASK)) {
        if (*next & ENTITY_ID_MASK) {
            Idx_t dense_idx = *next >> ENTITY_ID_SHIFT;
            next = (Entity_t *) cl->dense[dense_idx];
        } else {
            return NULL;
        }
    }

    return next;
}

// Flag sets
void ecs_cl_allow_purge(int id, bool allow)
{
    Component_List *cl = &component_lists[id];
    cl->no_purge = !allow;
}
void ecs_cl_keep_ordering(int id, bool keep)
{
    Component_List *cl = &component_lists[id];
    cl->keep_ordering = keep;
}

void ecs_purge_cls(void)
{
    for (int id = 0; id < 128; ++id) {
        Component_List *cl = &component_lists[id];
        if (!cl->initialized || cl->no_purge) {
            continue;
        }

        // We're purging everything

        // Dense pages
        for (unsigned int i = 0; i < cl->n_dense_pages; ++i) {
            free(cl->dense[i]);
            cl->dense[i] = NULL;
        }

        // Sparse entities
        for (unsigned int i = 0; i < MAX_ENTITIES; ++i) {
            cl->sparse[i] = SPARSE_NONE;
        }

        cl->n_dense_pages = 0;
        cl->count = 0;
    }

    for (unsigned int i = 0; i < MAX_ENTITIES; ++i) {
        entity_list[i] = (i + 1) << ENTITY_ID_SHIFT | 1;
        entity_component_lists[i*2]  = 0;
        entity_component_lists[i*2+1]= 0;
    }
    first_free_entity = 0;
}


// Entity manipulation
Entity_t ecs_new_entity(void)
{
    if (first_free_entity == MAX_ENTITIES) {
        FAIL_MESSAGE("Entity limit reached.");
    }

    Entity_t *entity = &entity_list[first_free_entity];
    Entity_t result = (first_free_entity << ENTITY_ID_SHIFT);

    if (first_free_entity == (*entity >> ENTITY_ID_SHIFT)) {
        printf("ECS error\n");
    }

    first_free_entity = *entity >> ENTITY_ID_SHIFT;
    *entity = ((MAX_ENTITIES - 1) << ENTITY_ID_SHIFT) | (*entity & ENTITY_VERSION_MASK);

    return result;
}

static void ecs_unreg_entity(Entity_t entity)
{
    // No error checking here
    Idx_t idx = entity >> ENTITY_ID_SHIFT;
    Entity_t *en = &entity_list[idx];

    printf("Deleting entity %d\n", idx);

    // Delete entity from list
    unsigned int version = entity & ENTITY_VERSION_MASK;
    version = (version == ENTITY_VERSION_MASK) ? 1 : version + 1;
    *en = (first_free_entity << ENTITY_ID_SHIFT) | version;
    first_free_entity = idx;
}


void ecs_delete_entity(Entity_t entity)
{
    Idx_t idx = entity >> ENTITY_ID_SHIFT;
    Entity_t *e = &entity_list[idx];

    if (*e >> ENTITY_ID_SHIFT != MAX_ENTITIES - 1) {
        // Entity doesn't exist
        return;
    }

    if ((*e & ENTITY_VERSION_MASK) != (entity & ENTITY_VERSION_MASK)) {
        // Entity already has a new version
        return;
    }

    // Delete its components
    uint64_t *components = &entity_component_lists[idx * 2];
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; components[i] && j < 64; ++j) {
            if (components[i] & (1ULL << j)) {
                ecs_remove_component(i*64 + j, entity);
            }
        }
    }
}

void ecs_init(void)
{
    for (unsigned int i = 0; i < MAX_ENTITIES; ++i) {
        entity_list[i] = (i + 1) << ENTITY_ID_SHIFT | 1;
    }
}
