#pragma once

#include "Type.hpp"

struct System_spark_fade
{
    using WriteAccess = ComponentFilter<
        ComponentSpark,
        ComponentColor
    >;

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        for(auto &&e : db.view(WriteAccess()))
        {
            auto &f_s = USAGI_COMPONENT(e, ComponentSpark);
            auto &f_c = USAGI_COMPONENT(e, ComponentColor);

            f_s.fade_time_left -= (float)rt.master_clock.elapsed();

            if(f_s.fade_time_left < 0)
                e.destroy();

            f_c.rgb = (f_s.fade_time_left / f_s.fade_time_total) *
                f_s.base_color;
        }
    }
};
