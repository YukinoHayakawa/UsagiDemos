#pragma once

#include <execution>
#include <algorithm>

#include <Usagi/Runtime/Service.hpp>

#include "Type.hpp"
#include "Service_master_clock.hpp"

struct System_physics
{
    using WriteAccess = ComponentFilter<ComponentPhysics, ComponentPosition>;

    Vector2f global_gravity { 0, -50 };

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        const auto dt = static_cast<float>(
            USAGI_SERVICE(rt, Service_master_clock).elapsed()
        );

        std::for_each(std::execution::par, db.begin(), db.end(), [&](auto &&p) {
            for(auto &&e : db.page_view(p, WriteAccess()))
            {
                auto &c_physics = USAGI_COMPONENT(e, ComponentPhysics);
                auto &c_pos = USAGI_COMPONENT(e, ComponentPosition);
                c_pos.position += c_physics.velocity * dt;
                c_physics.velocity +=
                    (c_physics.acceleration + global_gravity) * dt;
            }
        });
    }
};
