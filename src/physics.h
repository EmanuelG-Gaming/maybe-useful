#ifndef PHYSICS_H_
#define PHYSICS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

// Bro
#include <stdbool.h>

// Object pool and arena
#include "object_pool.h"
#include "arena.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables
#define PHYS_DEFAULT_LAYER 0
#define PHYS_PLAYER_LAYER 1
#define PHYS_ENEMY_LAYER 2

typedef enum Physics_Integrators {
    INTEGR_EULER,
    INTEGR_HEUN,
    INTEGR_RUNGE_KUTTA,
} Physics_Integrators;

typedef enum Collider_Shapes {
    COLLIDER_BOX = 0,
    COLLIDER_SPHERE,
} Collider_Shapes;

typedef enum Physics_Body_Behaviour {
    BODY_STATIC = 0,
    BODY_DYNAMIC,
} Physics_Body_Behaviour;

// Vectors
typedef struct Vec2f {
    float x, y;
} Vec2f;

typedef struct Vec3f {
    float x, y, z;
} Vec3f;

typedef Vec2f v2;
typedef Vec3f v3;

// Vector math shorthands
#define vec2_add(a, b) (a.x + b.x, a.y + b.y)

// Box (AABB)
typedef struct Collider_Box {
    v3 center;
    v3 sizes;
} Collider_Box;

// Sphere
typedef struct Collider_Sphere {
    v3 center;
    float radius;
} Collider_Sphere;

///////////////////////////////////////////////////////////////////////////////
// Physics
///////////////////////////////////////////////////////////////////////////////

// A material for a physics body
typedef struct {
    // How much energy should be lost with each "bounce" (1 is no energy being lost)
    float restitution;
    // 1/mass, a pre-computed value for collision resolution
    float invMass;

    float staticFriction;
    float dynamicFriction;
} Physics_Material;

typedef struct Physics_Body {
    // Accumulated force. Set to 0 at the start of the frame
    v3 force;
    
    v3 pos;
    v3 vel;
    v3 accel;

    Physics_Body_Behaviour type;

    // Which collision layer does this belong to?
    int layer;
    // Which layers should this object collide with?
int collision_mask;

    // Material parameters
    Physics_Material m;

    // Tagged union
    Collider_Shapes shape_type;
    union collider_shape {
        Collider_Box box;
        Collider_Sphere sphere;
    } shape;

    struct Physics_Body *next;
    bool in_use;
} Physics_Body;




// TODO: Maybe optimize the collision loop by only iterating over layers rather than the general world? I don't know, maybe not.

typedef struct {
    v3 AtoB;
    v3 BtoA;

    v3 normal;
    float depth;
    bool collided;

    Physics_Body *A;
    Physics_Body *B;
} Physics_Manifold;



typedef struct {
    Physics_Body *free_list;
    Physics_Body *pool;

    size_t count;
    size_t capacity;
} Physics_Bodies;

typedef struct {
    float delta_time;
    int64_t last_ticks;
    float time_accu;

    Physics_Bodies bodies;
} Physics_World;


void ph_body_init(Physics_Body *self);
void ph_body_create_box(Physics_Body *self, float width, float height, float depth);
void ph_body_create_sphere(Physics_Body *self, float radius);

void ph_body_set_mass(Physics_Body *self, float mass);
void ph_body_set_layer(Physics_Body *self, int layer);
void ph_body_set_colliding(Physics_Body *self, int layer_mask);
void ph_body_integrate(Physics_Body *self, float dt);



void ph_world_init(Physics_World *self, Arena *arena, size_t count);
void ph_world_free(Physics_World *self);

void ph_world_add(Physics_World *self, Physics_Body *obj);
void ph_world_remove(Physics_World *self, Physics_Body *obj);
void ph_world_removeIndex(Physics_World *self, unsigned int index);


void ph_world_update(Physics_World *self, float dt);
void ph_world_step(Physics_World *self, float dt);

void ph_world_compute_forces(Physics_World *self, float dt);
void ph_world_integrate(Physics_World *self, float dt);
void ph_world_handle_collisions(Physics_World *self, float dt);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // PHYSICS_H_
