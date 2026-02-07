#ifndef OBJECT_POOL_H_
#define OBJECT_POOL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "arena.h"

typedef struct Object_Pool_Node {
    int data;
    struct Object_Pool_Node *next;
} Object_Pool_Node;

typedef struct Object_Pool {
    Object_Pool_Node *free_list;
    Object_Pool_Node *pool;

    size_t count;
    size_t capacity;
} Object_Pool;


// TODO: fix

#define objpool_init(_pool, _count) \
    do { \
        _pool->free_list = _pool->pool; \
        _pool->capacity = (_count); \
        for (int i = 0; i < (_count) - 1; ++i) { \
            _pool->pool[i].next = &_pool->pool[i+1]; \
        } \
        _pool->pool[(_count)-1].next = NULL; \
    } while(0) \

#define objpool_clear(_pool, _to) (memset(_pool->pool, 0, (_to)))


#define objpool_add(_pool, _node) \
    do { \
        _node = _pool->free_list; \
        _pool->free_list = _node->next; \
        _pool->count++; \
    } while (0) \

#define objpool_remove(_pool, _node) \
    do { \
        _node->next = _pool->free_list; \
        _pool->free_list = _node; \
        _pool->count--; \
    } while (0) \

#define objpool_remove_index(_pool, _idx) \
    objpool_remove(_pool, _pool->pool[(_idx)])

/*
#define objpool_push(_pool, T) \
    do { \
        _pool->count++; \
    } while (0) \
*/


//
// This allocates an array inside the arena, for faster access in the linked list
//
void object_pool_init(Object_Pool* self, Arena* arena, size_t size);
void object_pool_deinit(Object_Pool* self);

Object_Pool_Node* object_pool_alloc(Object_Pool* self);
void object_pool_free(Object_Pool* self, Object_Pool_Node* node);

size_t object_pool_count(Object_Pool* self);

void object_pool_clear(Object_Pool* self);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // OBJECT_POOL_H_
