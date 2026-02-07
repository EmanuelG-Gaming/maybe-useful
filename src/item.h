#ifndef ITEM_H_
#define ITEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef uint32_t Idx_t;

typedef enum Item_Type {
    ITEM_NONE = -1,

    ITEM_COPPER, // Better than aluminium for wires
    ITEM_LEAD, // Might be poisonous to organisms!
    ITEM_COAL, // Welcome to the coal mines
    ITEM_GLASS, // There is no Walter White
    ITEM_POLYETHYLENE, // Cheap ahh plastic
    ITEM_POLYVINYL_CHLORIDE, // Used for pipes  
    ITEM_POLYTETRAFLUOROETHYLENE, // Frying pans
    ITEM_POLYBENZIMIDAZOLE, // Tough plastic used in aerospace engineering
    ITEN_POLYPHENYLENE_SULFIDE, // Used for cable insulation
    ITEM_POLYPNEUMOULTRAMICROSCOPICSILICOVOLCANOCONIOSIS, // Polymer of a respiratory disease
    ITEM_LLANFAIR_PWLLGWYNGYLL_GOGERY_CHWRN_DROWBLL_LLAN_TYSILIO_GOGO_GOCH, // Weird name for a real-life place
    ITEM_ANTIDISESTABLISHMENTARIANISM, // Anti-dis-establishment-arianism
} Item_Type;

typedef struct Item {
    Item_Type type;
    
    char *name;
    char *display_name;
} Item;

// Item stacks are easier to divide into 2 if their maximum capacity
// is a power of 2

typedef struct Item_Stack {
    Item *item;
    uint32_t count;
} Item_Stack;

typedef struct Inventory {
    Item_Stack *slots;
    uint32_t count;
    uint32_t capacity;
} Inventory;

// A slice of an inventory's slots, basically like a "view",
// that bounds a contiguous memory region.
// The data pointer can be incremented with pointer arithmetic.
typedef struct Inventory_Slice {
    Item_Stack *data;
    uint32_t length;
} Inventory_Slice;

/*
typedef struct Item_Registry {
    // *Insert hash table implementation here...*
} Item_Registry;
*/


///////////////////////////////////////////////////////////////////////////////
// Item
///////////////////////////////////////////////////////////////////////////////

void item_init(Item *self, const char *name);
Item item_create(const char *name);

///////////////////////////////////////////////////////////////////////////////
// Item stack
///////////////////////////////////////////////////////////////////////////////

void itemstack_init(Item_Stack *self, Item *item);
Item_Stack itemstack_create(Item *item, uint32_t count);

int itemstack_equals(Item_Stack *a, Item_Stack *b);
int itemstack_equals_item(Item_Stack *a, Item_Stack *b);
int itemstack_equals_count(Item_Stack *a, Item_Stack *b);

void itemstack_set_count(Item_Stack self, uint32_t count);
#define itemstack_add(self, count) (itemstack_set_count(self, self.count + (count)))
#define itemstack_remove(self, count) (itemstack_set_count(self, self.count - (count)))

void itemstack_copy(Item_Stack *src, Item_Stack *dest);

///////////////////////////////////////////////////////////////////////////////
// Inventory
///////////////////////////////////////////////////////////////////////////////

void inventory_init(Inventory *self);
Inventory inventory_create();

void inventory_free(Inventory *self);

void inventory_add_slot(Inventory *self, Item_Stack stack);
void inventory_set_stack(Inventory *self, Item_Stack stack, Idx_t idx);

void inventory_set_count(Inventory *self, uint32_t count, Idx_t idx);
void inventory_add_count(Inventory *self, uint32_t with, Idx_t idx);
void inventory_sub_count(Inventory *self, uint32_t with, Idx_t idx);

// Returns the first element found in the inventory,
// NULL if there isn't any.
Item_Stack* inventory_search(Inventory *self, Item *it);

// Uses qsort
void inventory_sort(Inventory *self);

void inventory_print(Inventory *self);

///////////////////////////////////////////////////////////////////////////////
// Registry
///////////////////////////////////////////////////////////////////////////////



#ifdef __cplusplus
} // extern "C"
#endif

#endif // ITEM_H_
