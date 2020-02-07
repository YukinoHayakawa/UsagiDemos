#pragma once

#include <random>

#include <Usagi/Math/Constant.hpp>

#include "Type.hpp"

inline Vector2f polarToCartesian(float magnitude, float angle_rad)
{
    return {
        magnitude * cos(angle_rad),
        magnitude * sin(angle_rad)
    };
}

struct System_fireworks_explode
{
    using WriteAccess = ComponentFilter<
        ComponentFireworks,
        ComponentSpark,
        ComponentPosition,
        ComponentPhysics,
        ComponentSprite,
        ComponentColor
    >;

    ArchetypeSpark spark;

    std::mt19937 gen { std::random_device()() };
    std::uniform_real_distribution<float> dis { 0, 2 * M_PI<float> };
    std::uniform_real_distribution<float> dis_v { 50, 150 };

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        for(auto &&e : db.view(ComponentFilter<
            ComponentFireworks,
            ComponentPosition
        >()))
        {
            auto &f_c = USAGI_COMPONENT(e, ComponentFireworks);
            auto &f_phy = USAGI_COMPONENT(e, ComponentPhysics);

            // f_c.time_to_explode -= 1.f / 60;
            if(f_phy.velocity.y() < 1)
            {
                for(auto i = 0; i < f_c.num_sparks; ++i)
                {
                    spark.val<ComponentSpark>();
                    // copy the rocket position
                    spark.val<ComponentPosition>() =
                        USAGI_COMPONENT(e, ComponentPosition);
                    spark.val<ComponentPhysics>().velocity =
                        polarToCartesian(dis_v(gen), dis(gen));
                    spark.val<ComponentSprite>().size = 5;
                    spark.val<ComponentColor>() =
                        USAGI_COMPONENT(e, ComponentColor);
                    db.create(spark);
                }
                e.destroy();
            }
        }
    }
};
