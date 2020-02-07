#pragma once

#include <random>

#include <Usagi/Core/Clock.hpp>

#include "Type.hpp"

struct System_fireworks_spawn
{
    using WriteAccess = ArchetypeFireworks::ComponentFilterT;

    std::mt19937 gen { std::random_device()() };
    std::uniform_real_distribution<float> dis { .1f, 1.f };
    std::uniform_real_distribution<float> dis_x { 100, 1820 };
    std::uniform_real_distribution<float> dis_v { 250, 300 };
    std::uniform_real_distribution<float> dis_color { 0, 255 };
    Clock timer;
    float wait_time_sec = dis(gen);

    ArchetypeFireworks fireworks;

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&svc, EntityDatabaseAccess &&db)
    {
        if(timer.now() > wait_time_sec)
            // for(auto i = 0; i < 17; ++i)
        {
            fireworks.val<ComponentFireworks>().num_sparks = 100;
            fireworks.val<ComponentFireworks>().time_to_explode = 2;
            fireworks.val<ComponentPosition>().position = { dis_x(gen), 0 };
            fireworks.val<ComponentPhysics>().velocity = { 0, dis_v(gen) };
            fireworks.val<ComponentPhysics>().acceleration = { 0, 0 };
            fireworks.val<ComponentSprite>().size = 15;
            fireworks.val<ComponentColor>().rgb =
                { dis_color(gen), dis_color(gen), dis_color(gen) };

            const EntityId e = db.create(fireworks);

            timer.reset();
            wait_time_sec = dis(gen);
        }
    }
};
