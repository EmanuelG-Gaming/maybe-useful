#include <stdio.h>

#include "mathf.h"
#include "mathf.c"

#include "arena.h"

#include "object_pool.c"
#include "object_pool.h"


typedef struct {
    char name[64];
    int x;
} State;

// Matrix testing
void mat_test()
{
    printf("Matrix testing\n"
           "//////////////\n");
    Mat4 d;

    Vec4 eye; vec4_set3(&eye, 1, 0, 0);
    Vec4 center; vec4_set3(&center, 0, 0, 0);
    Vec4 up; vec4_set3(&up, 0, 1, 0);

    mat4_ident(&d, 1);
    mat4_lookAt(&d, &eye, &center, &up);
    print_mat4(d);
}

int main()
{
    // Allocates the scratch arenas
    arena_system_init();

    Arena* arena = arena_alloc();

    Object_Pool pool;
    object_pool_init(&pool, arena, 100000);

    Object_Pool_Node* n = object_pool_alloc(&pool);
    n->data = 5;
    //object_pool_clear(&pool);

    printf("Data: %d\n", n->data);

    mat_test();
    /*
    State *s1 = arena_push_struct(arena, State);
    s1->x = 67;
    arena_print(arena);
    printf("%d\n", s1->x);
    */

    object_pool_deinit(&pool);
    arena_clear(arena);
    arena_release(arena);

    arena_system_deinit();
}
