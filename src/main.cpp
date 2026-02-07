#include "item.h"
#include "mathf.h"
#include "mathf_wrap.h"
#include "gamedev.h"
#include "gamedev.c"

#include "item.h"
#include "item.c"

#include "log.h"
#include "log.c"


#include <stdio.h>
#include <unistd.h>

typedef struct {
    float x, y;
} Position_Component;
#define POS_COMP 1

typedef struct {
    Entity_t ent;
} Object;

void example_task()
{
    printf("hello world!\n");
}

void mat_test()
{
    Mat f = Mat::FromTranslation(Vec{0, 3, 4});
    print_mat4(f);
}

void inventory_test()
{
    //log_frame_begin();

    Item itm = item_create("Polytetrafluoroethylene");

    Inventory inv = inventory_create();
    inventory_add_slot(&inv, itemstack_create(&itm, 68));

    inventory_print(&inv);
    inventory_free(&inv);

    log_frame_begin();
    for (int i = 0; i < 100; ++i) {
        log_info("a");
    }
    log_flush_level(LOG_INFO);
}

int main()
{
    ecs_init();

    // Component lists (position)
    ecs_cl_init(POS_COMP, Position_Component);

    for (int i = 0; i < 100; ++i) {
         Object obj;
         obj.ent = ecs_new_entity();
    
         Position_Component *comp = (Position_Component *) ecs_new_component(POS_COMP, obj.ent);
         comp->x = 5 + i;
         comp->y = 3 + i;
    }

    // Iterate over components
    //for (Position_Component *comp = (Position_Component *) ecs_cl_begin(POS_COMP); comp; comp = (Position_Component *) ecs_cl_next(POS_COMP, comp)) {
        //Position_Component *c = (Position_Component *) comp;
        //printf("Position(x:%f, y:%f)", c->x, c->y);
    //}
    Position_Component *c = (Position_Component *) ecs_cl_begin(POS_COMP);
    printf("Position(x:%f, y:%f)\n\n", c->x, c->y);

    // crash when calling ecl_cl_next()
    //c = (Position_Component *) ecs_cl_next(POS_COMP, c);
    //printf("Position(x:%f, y:%f)", c->x, c->y);

    inventory_test();

    return 0;
}
