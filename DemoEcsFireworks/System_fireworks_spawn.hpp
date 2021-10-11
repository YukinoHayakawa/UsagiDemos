#pragma once

#include <random>

#include <Usagi/Runtime/Service/Service.hpp>
#include <Usagi/Modules/Common/Time/Clock.hpp>

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
            USAGI_SERVICE(rt, Service_master_clock).last_frame_time()
        );

        while(time_pool > 0.f)
        {
            fireworks(C<ComponentFireworks>()).num_sparks = 200;
            fireworks(C<ComponentFireworks>()).time_to_explode = 2;
            fireworks(C<ComponentPosition>()).position = { dis_x(gen), 0 };
            fireworks(C<ComponentPhysics>()).velocity = { 0, dis_v(gen) };
            fireworks(C<ComponentPhysics>()).acceleration = { 0, 0 };
            fireworks(C<ComponentSprite>()).size = 10;
            fireworks(C<ComponentColor>()).rgb =
                { dis_color(gen), dis_color(gen), dis_color(gen) };
            db.insert(fireworks);
            time_pool -= 0.03f;
        }
    }
};
