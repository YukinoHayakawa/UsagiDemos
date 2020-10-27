#pragma once

#include <random>

#include <Usagi/Runtime/Service.hpp>
#include <Usagi/Module/Common/Time/Clock.hpp>

#include "Type.hpp"
#include "Service_master_clock.hpp"

struct System_fireworks_spawn
{
    using WriteAccess = ArchetypeFireworks::ComponentFilterT;

    std::mt19937 gen { std::random_device()() };
    std::uniform_real_distribution<float> dis { .05f, .1f };
    std::uniform_real_distribution<float> dis_x { 100, 1820 };
    std::uniform_real_distribution<float> dis_v { 250, 300 };
    std::uniform_real_distribution<float> dis_color { 0, 255 };

    ArchetypeFireworks fireworks;
    float time_pool = 0;

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        time_pool += static_cast<float>(
            USAGI_SERVICE(rt, Service_master_clock).elapsed()
        );

        while(time_pool > 0.f)
        {
            fireworks.val<ComponentFireworks>().num_sparks = 200;
            fireworks.val<ComponentFireworks>().time_to_explode = 2;
            fireworks.val<ComponentPosition>().position = { dis_x(gen), 0 };
            fireworks.val<ComponentPhysics>().velocity = { 0, dis_v(gen) };
            fireworks.val<ComponentPhysics>().acceleration = { 0, 0 };
            fireworks.val<ComponentSprite>().size = 10;
            fireworks.val<ComponentColor>().rgb =
            { dis_color(gen), dis_color(gen), dis_color(gen) };

            const EntityId e = db.create(fireworks);
            (void)e;
            time_pool -= 0.03f;
        }
    }
};
