#include "object_pool.h"
#include "arena.c"
#include "arena.h"

void object_pool_init(Object_Pool *self, Arena *arena, size_t size)
{
    if (size == 0 || !self) {
        return;
    }

    self->pool = arena_push_array(arena, Object_Pool_Node, size);
    if (!self->pool) {
        FAIL_MESSAGE("Failed to push array to arena!");
    }

    objpool_init(self, size);
}

Object_Pool_Node* object_pool_alloc(Object_Pool* self)
{
    if (!self->free_list) {
        FAIL_MESSAGE("There is no free list allocated!");
    }

    Object_Pool_Node *node;
    objpool_add(self, node);
    //self->free_list = new_node->next;

    //self->count++;
    return node;
}

void object_pool_free(Object_Pool *self, Object_Pool_Node *node)
{
    if (!self || !node) {
        return;
    }
    objpool_remove(self, node);

    /*
    node->next = self->free_list;
    self->free_list = node;

    self->count--;
    */
}

void object_pool_clear(Object_Pool *self)
{
    objpool_clear(self, self->capacity);
    /*
    for (int i = 0; i < self->capacity; ++i) {
        self->pool[i].data = 0;
    }
    */
}


void object_pool_deinit(Object_Pool* self)
{
    if (!self) {
        return;
    }
    self->pool = NULL;
    self->free_list = NULL;
    self->capacity = 0;
}

size_t object_pool_count(Object_Pool *self)
{
    return self->count;
}
