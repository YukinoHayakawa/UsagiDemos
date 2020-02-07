#pragma once

#include "Type.hpp"
#include "Service_master_clock.hpp"
#include "Runtime.hpp"

struct System_physics
{
    using WriteAccess = ComponentFilter<ComponentPosition, ComponentPhysics>;

    Vector2f global_gravity { 0, -50 };

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        const auto dt = static_cast<float>(
            USAGI_SERVICE(rt, Service_master_clock).elapsed()
        );

        auto range = db.view(WriteAccess());
        auto begin = range.begin();
        auto end = range.end();

        while(begin != end)
        {
            auto &&e = *begin;

            auto &c_physics = USAGI_COMPONENT(e, ComponentPhysics);
            auto &c_pos = USAGI_COMPONENT(e, ComponentPosition);
            c_pos.position += c_physics.velocity * dt;
            c_physics.velocity +=
                (c_physics.acceleration + global_gravity) * dt;

            ++begin;
        }
    }
};
