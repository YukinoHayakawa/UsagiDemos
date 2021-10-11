// Fireworks ECS Demo

// Fires random fireworks from the land. The fireworks fly into the sky for
// some distance and explode. The sparks fly away from the explosion center
// and gradually disappear.

// #define NDEBUG

// #define Service_master_clock___          Service_master_clock_default
// #define Service_graphics_gdi___          Service_graphics_gdi
// #define Service_stat___                  Service_stat
// #define Service_content_update_flag___   Service_content_update_flag
#include <Usagi/Modules/Runtime/Executive/SystemTaskList.hpp>
#include <Usagi/Modules/Runtime/Executive/AppHost.hpp>

#include "System_fireworks_spawn.hpp"
#include "System_fireworks_explode.hpp"
#include "System_sprite_render.hpp"
#include "System_physics.hpp"
#include "System_remove_out_of_bound.hpp"
#include "System_spark_fade.hpp"
#include "System_gdi_present.hpp"
#include "Service_master_clock.hpp"
#include "System_background_render.hpp"
#include "System_draw_stat.hpp"
#include "System_update_flag.hpp"

using EnabledSystems = SystemTaskList<
    System_fireworks_spawn    ,
    System_fireworks_explode  ,
    System_spark_fade         ,
    System_physics            ,
    System_remove_out_of_bound,
    System_background_render  ,
    System_sprite_render      ,
    System_draw_stat          ,
    System_gdi_present        ,
    System_update_flag
>;

struct RuntimeServices
    : Service_master_clock_default
    , Service_graphics_gdi
    , Service_stat
    , Service_content_update_flag
{
};

using App = AppHost<RuntimeServices, EnabledSystems>;

struct SystemUpdateObserver
{
    StatFields &stat;
    Clock timer;

#define OBSERVE_SYSTEM(sys, stat_field) \
    void operator()(sys &, [[maybe_unused]] auto val) \
    { \
        stat.stat_field = timer.tick(); \
    } \
/**/

    OBSERVE_SYSTEM(System_fireworks_spawn, time_spawn)
    OBSERVE_SYSTEM(System_fireworks_explode, time_explode)
    OBSERVE_SYSTEM(System_spark_fade, time_fade)
    OBSERVE_SYSTEM(System_physics, time_physics)
    OBSERVE_SYSTEM(System_remove_out_of_bound, time_remove)
    OBSERVE_SYSTEM(System_background_render, time_clear)

    void operator()(System_sprite_render &, auto val)
    {
        stat.sprite_count = val;
        stat.time_render = timer.tick();
    }

    OBSERVE_SYSTEM(System_draw_stat, time_stat)
    OBSERVE_SYSTEM(System_gdi_present, time_present)

    // The system is passed as a l-value reference. A binding to r-value
    // reference makes this overload a worse candidate due to the implicit
    // conversion + reference collapsing. Therefore this can serve as a
    // fallback. Other overloads must use l-value references for the first
    // parameter.
    void operator()(auto &&, auto val)
    {
    }
};

int main()
{
    App demo { "demo_fireworks" };
    auto &svc = demo.services();

    while(true)
    {
        auto &stat = USAGI_SERVICE(svc, Service_stat);
        SystemUpdateObserver observer {
            .stat = stat
        };
        USAGI_SERVICE(svc, Service_master_clock).tick();

        if(!run_game()) break;
        stat.time_input = observer.timer.tick();

        demo.update(observer);
    }

    return 0;
}
