// Fireworks ECS Demo

// Fires random fireworks from the land. The fireworks fly into the sky for
// some distance and explode. The sparks fly away from the explosion center
// and gradually disappear.

#define NDEBUG

#include <Usagi/Utility/Utf8Main.hpp>
#include <Usagi/Experimental/v2/Game/_detail/EntityDatabaseAccessExternal.hpp>

#include "System_fireworks_spawn.hpp"
#include "System_fireworks_explode.hpp"
#include "System_sprite_render.hpp"
#include "System_physics.hpp"
#include "System_remove_out_of_bound.hpp"
#include "System_spark_fade.hpp"
#include "System_gdi_present.hpp"
#include "Service_master_clock.hpp"
#include "System_background_render.hpp"

#define UPDATE_SYSTEM(sys) \
    sys.update(rt, EntityDatabaseAccessExternal< \
        Database, \
        ComponentAccessSystemAttribute<decltype(sys)> \
    >(&db)) \
/**/

int usagi_main(const std::vector<std::string> &args)
{
    Database db;

    System_fireworks_spawn      sys_spawn;
    System_fireworks_explode    sys_explode;
    System_spark_fade           sys_fade;
    System_physics              sys_physics;
    System_remove_out_of_bound  sys_remove_oob;
    System_background_render    sys_bg;
    System_sprite_render        sys_render;
    System_gdi_present          sys_present;

    using namespace std::chrono_literals;

    struct RuntimeServices
        : Service_master_clock_default
        , Service_graphics_gdi
    {
    } rt;

    while(true)
    {
        USAGI_SERVICE(rt, Service_master_clock).tick();

        UPDATE_SYSTEM(sys_spawn);
        UPDATE_SYSTEM(sys_explode);
        UPDATE_SYSTEM(sys_fade);
        UPDATE_SYSTEM(sys_physics);
        UPDATE_SYSTEM(sys_remove_oob);
        UPDATE_SYSTEM(sys_bg);
        UPDATE_SYSTEM(sys_render);
        UPDATE_SYSTEM(sys_present);
    }

    return 0;
}
