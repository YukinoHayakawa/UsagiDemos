#pragma once

#include <random>
#include <execution>

#include <Usagi/Module/Common/Math/Constant.hpp>

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

    using Filter = ComponentFilter<
        ComponentFireworks,
        ComponentPosition
    >;

    std::mt19937 gen { std::random_device()() };
    std::uniform_real_distribution<float> dis { 0, 2 * M_PI<float> };
    std::uniform_real_distribution<float> dis_v { 50, 150 };
    std::uniform_real_distribution<float> dis_ft { 1, 2.5f };

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        // bug: do not execute parallelly. entity creation will not be handled correctly.
        std::for_each(std::execution::seq, db.begin(), db.end(), [&](auto &&p) mutable {
            static thread_local ArchetypeSpark spark;

            auto range = db.page_view(p, Filter());
            auto begin = range.begin();
            auto end = range.end();

            for(; begin != end; ++begin)
            {
                assert(begin != range.end());
                assert(end == range.end());

                auto e = *begin;
                const auto num_sparks =
                    USAGI_COMPONENT(e, ComponentFireworks).num_sparks;

                // f_c.time_to_explode -= 1.f / 60;
                if(USAGI_COMPONENT(e, ComponentPhysics).velocity.y() < 10)
                {
                    for(auto i = 0; i < num_sparks; ++i)
                    {
                        auto &f_c = USAGI_COMPONENT(e, ComponentColor);

                        spark.val<ComponentSpark>().fade_time_total =
                        spark.val<ComponentSpark>().fade_time_left =
                            dis_ft(gen);
                        spark.val<ComponentSpark>().base_color = f_c.rgb;

                        // copy the rocket position & color
                        spark.val<ComponentPosition>() =
                            USAGI_COMPONENT(e, ComponentPosition);
                        spark.val<ComponentColor>() =
                            USAGI_COMPONENT(e, ComponentColor);

                        // set particle props
                        spark.val<ComponentPhysics>().velocity =
                            polarToCartesian(dis_v(gen), dis(gen));
                        spark.val<ComponentSprite>().size = 5;

                        db.insert(spark);
                        // db.create may invalidate the entity view
                        e = *begin;
                    }
                    e.destroy();
                }
            }
        });
    }
};
