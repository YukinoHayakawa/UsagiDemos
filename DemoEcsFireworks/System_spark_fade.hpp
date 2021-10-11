#pragma once

#include <execution>

#include <Usagi/Modules/Common/Math/Bezier.hpp>

#include "Type.hpp"

struct System_spark_fade
{
    // required by entity.destroy()
    using WriteAccess = AllComponents;
    // Actually used components:
    using AccessedComponents = ComponentFilter<
        ComponentSpark,
        ComponentColor
    >;

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        const auto dt = static_cast<float>(
            USAGI_SERVICE(rt, Service_master_clock).last_frame_time()
        );

        std::for_each(std::execution::par, db.begin(), db.end(), [&](auto &&p) {
            db.page_view(p, ComponentFilter<>());
            for(auto &&e : db.page_view(p, AccessedComponents()))
            {
                auto &f_s = USAGI_COMPONENT(e, ComponentSpark);
                auto &f_c = USAGI_COMPONENT(e, ComponentColor);

                f_s.fade_time_left -= dt;
                if(f_s.fade_time_left < 0)
                {
                    e.destroy();
                }
                else
                {
                    const auto t = cssCubicBezier(
                        0.215f, 0.61f, 0.355f, 1.f,
                        f_s.fade_time_left / f_s.fade_time_total
                    );
                    f_c.rgb = t * f_s.base_color;
                }
            }
        });
    }
};
