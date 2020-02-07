#pragma once

#include "Type.hpp"

struct System_physics
{
    using WriteAccess = ComponentFilter<ComponentPosition, ComponentPhysics>;

    Vector2f global_gravity { 0, -50 };

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        auto dt = rt.master_clock.elapsed();
        // float dt = 1.f / 16;
        for(auto &&e : db.view(WriteAccess()))
        {
            auto &c_physics = USAGI_COMPONENT(e, ComponentPhysics);
            auto &c_pos = USAGI_COMPONENT(e, ComponentPosition);
            c_pos.position += c_physics.velocity * dt;
            c_physics.velocity +=
                (c_physics.acceleration + global_gravity) * dt;
        }
    }
};
