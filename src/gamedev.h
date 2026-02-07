#ifndef SCENE_H_
#define SCENE_H_

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// Things that you may use in gamedev:
// event system, scene system, timing system.
//
// The timing system requires a game loop, which can be described
// using a while (true) loop.
///////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "object_pool.h"

typedef uint32_t Entity_t;
typedef uint32_t Idx_t;

// Multiples of 2
#define EVENT_START_SCENE 4
#define EVENT_END_SCENE 8


#define MAX_ENTITIES 0x00100000UL
#define ENTITY_VERSION_MASK 0xFFF
#define ENTITY_ID_SHIFT 12
#define ENTITY_ID_MASK 0xFFFFF000

#define MAX_COMPONENT_LISTS 128


typedef struct Event_t {
    unsigned int type;
    int param1, param2;
} Event_t;

typedef struct Event_Callback {
    // All function pointers technically have the same size.
    union {
        void (*input)(void *arg, Event_t *ev);
        void (*update)(void *arg);
    };
    void *arg;
    int type_mask;
} Event_Callback;

typedef struct Event_Callbacks {
    Event_Callback *data;
    uint32_t count;
    uint32_t capacity;
} Event_Callbacks;


typedef void (*Task_Consumer)();
typedef struct Timed_Task {
    Task_Consumer cons;
    float snapshot; // The snapshot in time at which the action happened
    float delay;
    struct Timed_Task *next;
} Timed_Task;


typedef struct Timing_System {
    // Object pool eeeeee
    Timed_Task *free_list;
    Timed_Task *pool;

    size_t count;
    size_t capacity;
} Timing_System;


// Global variables woo

extern const char *g_scene_name;

///////////////////////////////////////////////////////////////////////////////
// Timing system
///////////////////////////////////////////////////////////////////////////////

void time_init(Timing_System *self);
void time_deinit(Timing_System *self);


Timed_Task* time_schedule(Timing_System *self, float delay, Task_Consumer cons);
void time_repeat_delayed(Timing_System *self,
    float initial_delay, float interval, unsigned int count, Task_Consumer cons);

void time_remove(Timing_System *self, Timed_Task *task);

// Polling
void time_run_due_tasks(Timing_System *self);

float time_time(void);


#define time_repeat(timing, interval, count, cons) (time_repeat_delayed(timing, 0, interval, count, cons))

///////////////////////////////////////////////////////////////////////////////
// Event system
///////////////////////////////////////////////////////////////////////////////

void events_notify_update(Event_Callbacks *subscribers, Event_t *ev);
void events_notify_input(Event_Callbacks *subscribers, Event_t *ev);


// Called whenever a scene starts
void events_start_scene(void);

// Called whenever a scene ends
void events_end_scene(void);

///////////////////////////////////////////////////////////////////////////////
// Scene system
///////////////////////////////////////////////////////////////////////////////

void scene_switch(const char *to);


///////////////////////////////////////////////////////////////////////////////
// Entity component system (ECS)
///////////////////////////////////////////////////////////////////////////////

void ecs_cl_init_sz(int id, unsigned int element_size);
#define ecs_cl_init(id, T) (ecs_cl_init_sz((id), sizeof(T)))

void ecs_cl_deinit(int id);
void ecs_purge_cls(void);

void ecs_init(void);

void* ecs_get_component(int id, Entity_t entity);
// Returns NULL if there's no entity
void* ecs_get_component_nullable(int id, Entity_t entity);

void* ecs_new_component(int id, Entity_t entity);
void ecs_remove_component(int id, Entity_t entity);

Idx_t ecs_cl_count(int id);
void* ecs_cl_at(int id, Idx_t index);

void* ecs_cl_begin(int id);
void* ecs_cl_next(int id, void *iter);

// Flag sets
void ecs_cl_allow_purge(int id, bool allow);
void ecs_cl_keep_ordering(int id, bool keep);
void ecs_cl_ordered_clean(int id);

// Entity manipulation
Entity_t ecs_new_entity(void);
void ecs_delete_entity(Entity_t entity);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SCENE_H_
