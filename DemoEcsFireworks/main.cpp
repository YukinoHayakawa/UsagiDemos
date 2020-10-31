// Fireworks ECS Demo

// Fires random fireworks from the land. The fireworks fly into the sky for
// some distance and explode. The sparks fly away from the explosion center
// and gradually disappear.

#ifdef _DEBUG
#pragma comment(lib, "fmtd.lib")
#else
#pragma comment(lib, "fmt.lib")
#endif

// #define NDEBUG

#include <Usagi/Entity/detail/EntityDatabaseAccessExternal.hpp>

// #define Service_master_clock___          Service_master_clock_default
// #define Service_graphics_gdi___          Service_graphics_gdi
// #define Service_stat___                  Service_stat
// #define Service_content_update_flag___   Service_content_update_flag

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


using EnabledComponents = SystemComponentUsage<
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


/*
using EnabledComponents = ComponentFilter<
    ComponentFireworks,
    ComponentSpark,
    ComponentPosition,
    ComponentPhysics,
    ComponentSprite,
    ComponentColor
>;*/


// todo infer components from systems
using App = EnabledComponents::apply<ResumableApp>;

#define UPDATE_SYSTEM(sys) \
    sys.update(rt, EntityDatabaseAccessExternal< \
        App::DatabaseT, \
        ComponentAccessSystemAttribute<decltype(sys)> \
    >(&db)) \
/**/

int main()
{
    App demo { "demo_fireworks" };
    auto &db = demo.database();

    /*
    const auto reserve = [](auto &alloc) {
        alloc.storage().allocator().reserve(1ull << 32);
    };

    db.init_entity_page_storage(reserve);
    db.init_component_storage(reserve);
    */

    System_fireworks_spawn      sys_spawn;
    System_fireworks_explode    sys_explode;
    System_spark_fade           sys_fade;
    System_physics              sys_physics;
    System_remove_out_of_bound  sys_remove_oob;
    System_background_render    sys_bg;
    System_sprite_render        sys_render;
    System_draw_stat            sys_stat;
    System_gdi_present          sys_present;
    System_update_flag          sys_update_flag_test;

    using namespace std::chrono_literals;

    struct RuntimeServices
        : Service_master_clock_default
        , Service_graphics_gdi
        , Service_stat
        , Service_content_update_flag
    {
    } rt;

    while(true)
    {
        USAGI_SERVICE(rt, Service_master_clock).tick();
        auto &stat = USAGI_SERVICE(rt, Service_stat);

        Clock timer;

        if(!run_game()) break;
        stat.time_input = timer.tick();

        UPDATE_SYSTEM(sys_spawn);
        stat.time_spawn = timer.tick();

        UPDATE_SYSTEM(sys_explode);
        stat.time_explode = timer.tick();

        UPDATE_SYSTEM(sys_fade);
        stat.time_fade = timer.tick();

        UPDATE_SYSTEM(sys_physics);
        stat.time_physics = timer.tick();

        UPDATE_SYSTEM(sys_remove_oob);
        stat.time_remove = timer.tick();

        UPDATE_SYSTEM(sys_bg);
        stat.time_clear = timer.tick();

        stat.sprite_count = UPDATE_SYSTEM(sys_render);
        stat.time_render = timer.tick();

        UPDATE_SYSTEM(sys_stat);
        stat.time_stat = timer.tick();

        UPDATE_SYSTEM(sys_present);
        stat.time_present = timer.tick();

        UPDATE_SYSTEM(sys_update_flag_test);

        db.reclaim_pages();
    }

    return 0;
}
