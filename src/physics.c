#include "physics.h"
#include "arena.h"
#include "object_pool.h"
#include "stdlib.h"
#include "string.h"
#include <strings.h>

static const Physics_Integrators integration_method = INTEGR_EULER;

// Gravitational acceleration on Earth
static v3 gravity = { 0.0, -9.81, 0.0 };

///////////////////////////////////////////////////////////////////////////////
// Static (internal) functions

void _integrate_euler(Physics_Body *self, float dt)
{
    self->accel.x = self->force.x * self->m.invMass;
    self->accel.y = self->force.y * self->m.invMass;
    self->accel.z = self->force.z * self->m.invMass;

    self->vel.x += self->accel.x * dt;
    self->vel.y += self->accel.y * dt;
    self->vel.z += self->accel.z * dt;

    self->pos.x += self->vel.x * dt;
    self->pos.y += self->vel.y * dt;
    self->pos.z += self->vel.z * dt;
}

void _integrate_heun(Physics_Body *self, float dt)
{
    (void) self;
    (void) dt;
}

///////////////////////////////////////////////////////////////////////////////
// Colliders
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Physics bodies
///////////////////////////////////////////////////////////////////////////////

void ph_body_init(Physics_Body *self)
{
    self->force = (v3){0};
    self->pos = self->vel = self->accel = (v3){0};

    self->layer = PHYS_DEFAULT_LAYER;

    // Material properties
    self->m.restitution = 1;
    self->m.invMass = 1;
    self->m.dynamicFriction = 0.001;
    self->m.staticFriction = 0;
}

void ph_body_create_box(Physics_Body *self, float width, float height, float depth)
{
    ph_body_init(self);

    self->shape_type = COLLIDER_BOX;
    self->shape.box.center = (v3){0};
    self->shape.box.sizes = (v3) { width, height, depth };
}

void ph_body_create_sphere(Physics_Body *self, float radius)
{
    ph_body_init(self);

    self->shape_type = COLLIDER_SPHERE;
    self->shape.sphere.center = (v3){0};
    self->shape.sphere.radius = radius;
}

void ph_body_set_mass(Physics_Body *self, float mass)
{
    self->m.invMass = 1 / mass;
}

void ph_body_set_layer(Physics_Body *self, int layer)
{
    self->layer = (1 << layer);
}

void ph_body_set_colliding(Physics_Body *self, int layer_mask)
{
    self->collision_mask = (1 << layer_mask);
}

void ph_body_integrate(Physics_Body *self, float dt)
{
    switch (integration_method) {
        case INTEGR_EULER: _integrate_euler(self, dt); break;
        case INTEGR_HEUN: _integrate_heun(self, dt); break;
        case INTEGR_RUNGE_KUTTA: break;
        default: break;
    }
}


///////////////////////////////////////////////////////////////////////////////
// Physics world
///////////////////////////////////////////////////////////////////////////////

void ph_world_init(Physics_World *self, Arena *arena, size_t count)
{
    self->bodies.pool = arena_push_array(arena, Physics_Body, count);
    if (!self->bodies.pool) {
        FAIL_MESSAGE("Failed to push physics object pool to arena!");
    }

    objpool_init(self->bodies, count);
}


void ph_world_free(Physics_World *self)
{
    self->bodies.count = 0;
}

void ph_world_remove(Physics_World *self, Physics_Body *obj)
{
    objpool_remove(self->bodies, obj);
}

void ph_world_removeIndex(Physics_World *self, unsigned int index)
{
    objpool_remove_index(self->bodies, index);
}

void ph_world_clear(Physics_World *self)
{
    objpool_clear(self->bodies, self->bodies.count);
}

void ph_world_step(Physics_World *self, float dt)
{
    for (int i = 0; i < self->bodies.count; ++i) {
        self->bodies.pool[i].force = (v3){0};
    }
    ph_world_compute_forces(self, dt);
    ph_world_integrate(self, dt);
    ph_world_handle_collisions(self, dt);
}

void ph_world_compute_forces(Physics_World *self, float dt)
{
    (void) dt;
    
    for (int i = 0; i < self->bodies.count; ++i) {
        float mass = 1 / self->bodies.pool[i].m.invMass;

        // Gravitational accelerations G = mg
        self->bodies.pool[i].force = vec3_scale(gravity, mass);

        // After that, you add forces together
    }
}

void ph_world_integrate(Physics_World *self, float dt)
{
    for (int i = 0; i < self->bodies.count; ++i) {
        ph_body_integrate(&self->bodies.pool[i], dt);
    }
}

void ph_world_handle_collisions(Physics_World *self, float dt)
{
    int i, j;
    for (i = 0; i < self->bodies.count; ++i) {
        Physics_Body *body = &self->bodies.pool[i];
        
        for (j = 0; j < self->bodies.count; ++j) {
            if (i == j) continue;

            // Broad-phase collision checks
        }
    }
}

unsigned int ph_world_obj_count(Physics_World *self)
{
    return self->bodies.count;
}
