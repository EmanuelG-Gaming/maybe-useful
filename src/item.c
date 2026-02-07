#include "item.h"
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// Item
///////////////////////////////////////////////////////////////////////////////

void item_init(Item *self, const char *name)
{
    self->type = ITEM_NONE;
    self->name = self->display_name = (char *) name;
}

Item item_create(const char *name)
{
    Item ret;
    item_init(&ret, name);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Item stack
///////////////////////////////////////////////////////////////////////////////

void itemstack_init(Item_Stack *self, Item *item)
{
    self->count = 0;
    self->item = item;
}

Item_Stack itemstack_create(Item *item, uint32_t count)
{
    Item_Stack ret;
    ret.item = item;
    ret.count = count;

    return ret;
}

int itemstack_equals(Item_Stack *a, Item_Stack *b)
{
    return (a->item == b->item && a->count == b->count);
}
int itemstack_equals_item(Item_Stack *a, Item_Stack *b)
{
    return (a->item == b->item);
}
int itemstack_equals_count(Item_Stack *a, Item_Stack *b)
{
    return (a->count == b->count);
}


void itemstack_set_count(Item_Stack self, uint32_t count)
{
    if (self.count < count) {
        self.count = 0;
        return;
    }
    self.count = count;
}

void itemstack_copy(Item_Stack *src, Item_Stack *dest)
{
    dest->count = src->count;
    dest->item = src->item;
}

///////////////////////////////////////////////////////////////////////////////
// Inventory
///////////////////////////////////////////////////////////////////////////////

void inventory_init(Inventory *self)
{
    self->count = 0;
    self->capacity = 0;
    self->slots = NULL;
}

void inventory_free(Inventory *self)
{
    free(self->slots);
}


Inventory inventory_create()
{
    Inventory ret;
    inventory_init(&ret);
    return ret;
}


void inventory_add_slot(Inventory *self, Item_Stack stack)
{
    if (self->count >= self->capacity) {
        //unsigned long long count = self->capacity ? self->capacity * 2 : 1;
        unsigned long long count = 256;
        self->slots = (Item_Stack *) malloc(count * sizeof(Item_Stack));
    }


    self->slots[self->count] = stack;
    self->count++;
}

void inventory_set_stack(Inventory *self, Item_Stack stack, Idx_t idx)
{
    if (idx < 0 || idx >= self->count) {
        return;
    }
    self->slots[idx] = stack;
}


void inventory_set_count(Inventory *self, uint32_t count, Idx_t idx)
{
    itemstack_set_count(self->slots[idx], count);
}

void inventory_add_count(Inventory *self, uint32_t count, Idx_t idx)
{
    itemstack_add(self->slots[idx], count);
}

void inventory_sub_count(Inventory *self, uint32_t count, Idx_t idx)
{
    itemstack_remove(self->slots[idx], count);
}

Item_Stack* inventory_search(Inventory *self, Item *it)
{
    for (int i = 0; i < self->count; ++i) {
        if (self->slots[i].item == it) {
            return &self->slots[i];
        }
    }
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Sorting
///////////////////////////////////////////////////////////////////////////////

typedef int (*Sort_Predicate)(const void *a, const void *b);

// Sorts from lowest to highest
static inline int Itemstack_Sort_Ascending_u32(const void *a, const void *b)
{ 
    return (((Item_Stack*)a)->count - ((Item_Stack*)b)->count);
}
// Sorts from highest to lowest
static inline int Itemstack_Sort_Descending_u32(const void *a, const void *b)
{
    return (((Item_Stack*)b)->count - ((Item_Stack*)a)->count);
}

static void _inventory_sort(Inventory *self, Sort_Predicate _cmp)
{
    qsort(self->slots, self->count, sizeof(Item_Stack), _cmp);
}

void inventory_sort(Inventory *self)
{
    _inventory_sort(self, Itemstack_Sort_Ascending_u32);
}

void inventory_sort_desc(Inventory *self)
{
    _inventory_sort(self, Itemstack_Sort_Descending_u32);
}

void inventory_print(Inventory *self)
{
    printf("INVENTORY:\n"
           "------------------------\n");

    for (int i = 0; i < self->count; ++i) {
        printf("Item: %s, count: %d\n",
                self->slots[i].item->display_name,
                self->slots[i].count);
    }

    printf("------------------------\n");
}
