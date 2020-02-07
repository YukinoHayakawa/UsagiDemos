#define NDEBUG

#include <Usagi/Utility/Utf8Main.hpp>
#include <Usagi/Math/Matrix.hpp>
#include <Usagi/Experimental/v2/Game/Entity/Archetype.hpp>
#include <Usagi/Experimental/v2/Game/Database/EntityDatabase.hpp>
// #include <Usagi/Experimental/v2/Game/System.hpp>
// #include <Usagi/Experimental/v2/Executive/>
#include <Usagi/Core/Clock.hpp>
#include <Usagi/Experimental/v2/Game/_detail/ComponentFilter.hpp>
#include <Usagi/Core/Logging.hpp>
#include <Usagi/Experimental/v2/Game/_detail/EntityDatabaseAccessExternal.hpp>
#include <Usagi/Experimental/v2/Game/_detail/ComponentAccessSystemAttribute.hpp>
#include <Usagi/Math/Constant.hpp>

using namespace usagi;

// Fireworks ECS Demo

// Fires random fireworks from the land. The fireworks fly into the sky for
// some distance and explode. The sparks fly away from the explosion center
// and gradually disappear.

#include <random>

struct ComponentFireworks
{
    int num_sparks;
    float time_to_explode;
};
static_assert(Component<ComponentFireworks>);

struct ComponentSpark
{
    float brightness;
    float fade_speed;
};
static_assert(Component<ComponentSpark>);

struct ComponentPosition
{
    Vector2f position;
};
static_assert(Component<ComponentPosition>);

struct ComponentPhysics
{
    Vector2f velocity;
    Vector2f acceleration;
};
static_assert(Component<ComponentPhysics>);

struct ComponentSprite
{
    int size;
};
static_assert(Component<ComponentSprite>);

struct ComponentColor
{
    int r, g, b;
};
static_assert(Component<ComponentColor>);

using ArchetypeFireworks = Archetype<
    ComponentFireworks,
    ComponentPosition,
    ComponentPhysics,
    ComponentSprite,
    ComponentColor
>;

using ArchetypeSpark = Archetype<
    ComponentSpark,
    ComponentPosition,
    ComponentPhysics,
    ComponentSprite,
    ComponentColor
>;

// todo infer components from systems
using Database = EntityDatabase<
    16,
    ComponentFilter<
        ComponentFireworks,
        ComponentSpark,
        ComponentPosition,
        ComponentPhysics,
        ComponentSprite,
        ComponentColor
    >
>;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct SystemFireworksSpawn
{
    using WriteAccess = ArchetypeFireworks::ComponentFilterT;

    std::mt19937 gen { std::random_device()() };
    std::uniform_real_distribution<float> dis { .5f, 1.5f };
    std::uniform_real_distribution<float> dis_x { 100, 1820 };
    std::uniform_real_distribution<float> dis_v { 250, 300 };
    std::uniform_int_distribution<> dis_color { 0, 255 };
    Clock timer;
    float wait_time_sec = dis(gen);

    ArchetypeFireworks fireworks;

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&svc, EntityDatabaseAccess &&db)
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
            fireworks.val<ComponentColor>().r = dis_color(gen);
            fireworks.val<ComponentColor>().g = dis_color(gen);
            fireworks.val<ComponentColor>().b = dis_color(gen);

            const EntityId e = db.create(fireworks);

            timer.reset();
            wait_time_sec = dis(gen);
        }
    }
};

struct SystemPhysics
{
    using WriteAccess = ComponentFilter<ComponentPosition, ComponentPhysics>;

    Vector2f global_gravity { 0, -50 };

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        // auto dt = rt.masterClock.elapsed();
        float dt = 1.f / 16;
        for(auto &&e : db.view(WriteAccess()))
        {
            auto &c_physics = USAGI_COMPONENT(e, ComponentPhysics);
            auto &c_pos = USAGI_COMPONENT(e, ComponentPosition);
            c_pos.position += c_physics.velocity * dt;
            c_physics.velocity +=
                (c_physics.acceleration + global_gravity) * dt;
        }
    }
};

struct SystemRemovePhysics
{
    using ReadAccess = ComponentFilter<ComponentPosition>;
    using WriteAccess = ComponentFilter<ComponentPhysics>;

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        for(auto &&e : db.view(WriteAccess()))
        {
            auto &c_pos = USAGI_COMPONENT(e, ComponentPosition);
            if(c_pos.position.y() < 0)
                e.template removeComponent<ComponentPhysics>();
        }
    }
};

Vector2f polarToCartesian(float magnitude, float angle_rad)
{
    return {
        magnitude * cos(angle_rad),
        magnitude * sin(angle_rad)
    };
}

struct SystemFireworksExplode
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

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
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
                    db.create(spark); // <- bug create entities during iteration will invalidate the iterators
                }
                e.destroy();
            }
        }
    }
};

struct SystemSpriteRendering
{
    using ReadAccess = ComponentFilter<
        ComponentPosition,
        ComponentSprite,
        ComponentColor
    >;

    // EntityDB is a concept that covers all instantiations of the
    // database template
    // EntityDatabaseAccess adds layer of permission checking. if the system
    // does not have access to some components, the access is denied.
    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        // https://stackoverflow.com/questions/1937163/drawing-in-a-win32-console-on-c

        // Get window handle to console, and device context
        HWND console_handle = GetConsoleWindow();
        HDC hdc = GetDC(console_handle);

        SelectObject(hdc, GetStockObject(WHITE_PEN));
        MoveToEx(hdc, 0, 1080, nullptr);
        LineTo(hdc, 1920, 1080);
        LineTo(hdc, 1920, 0);

        SelectObject(hdc, GetStockObject(BLACK_BRUSH));
        Rectangle(hdc, 0, 0, 1920, 1080);

        SelectObject(hdc, GetStockObject(DC_PEN));
        SelectObject(hdc, GetStockObject(DC_BRUSH));
        for(auto &&e : db.view(ReadAccess()))
        {
            auto &pos = USAGI_COMPONENT(e, ComponentPosition);
            auto &sprite = USAGI_COMPONENT(e, ComponentSprite);
            auto &color = USAGI_COMPONENT(e, ComponentColor);

            const auto rgb = RGB(color.r, color.g, color.b);
            SetDCPenColor(hdc, rgb);
            SetDCBrushColor(hdc, rgb);
            Rectangle(hdc,
                (int)pos.position.x(),
                1080 - (int)pos.position.y(),
                (int)pos.position.x()+ sprite.size,
                1080 - ((int)pos.position.y() + sprite.size)
            );
        }

        ReleaseDC(console_handle, hdc);
    }
};

#include <thread>
#include <chrono>

int usagi_main(const std::vector<std::string> &args)
{
    Database db;

    int rt = 0;

    SystemFireworksSpawn sys_spawn;
    SystemFireworksExplode sys_explode;
    SystemPhysics sys_physics;
    SystemRemovePhysics sys_remove_physics;
    SystemSpriteRendering sys_render;

    // // Build the execution graph
    // auto task = exec
    //     .run<FireworksSpawnSystem>()
    //     .then<FireworksExplodeSystem>()
    //     .then<SparkFadingSystem, PhysicsSystem>()
    //     .then<SpriteRenderingSystem>();

    using namespace std::chrono_literals;

    while(true)
    {
        sys_spawn.update(rt, EntityDatabaseAccessExternal<
            Database,
            ComponentAccessSystemAttribute<SystemFireworksSpawn>
        >(&db));
        sys_explode.update(rt, EntityDatabaseAccessExternal<
            Database,
            ComponentAccessSystemAttribute<SystemFireworksExplode>
        >(&db));
        sys_physics.update(rt, EntityDatabaseAccessExternal<
            Database,
            ComponentAccessSystemAttribute<SystemPhysics>
        >(&db));
        sys_remove_physics.update(rt, EntityDatabaseAccessExternal<
            Database,
            ComponentAccessSystemAttribute<SystemRemovePhysics>
        >(&db));
        sys_render.update(rt, EntityDatabaseAccessExternal<
            Database,
            ComponentAccessSystemAttribute<SystemSpriteRendering>
        >(&db));
        // std::this_thread::sleep_for(16ms);
    }

    return 0;
}

/*


struct SystemSparkFading
{
    using WriteAccess = ComponentFilter<ComponentSpark>;

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        auto view = db.view<write_access_t>;

        for(auto &&s : view)
        {
            s<ComponentSpark>().brightness -= s<ComponentSpark>().fade_speed *
                svc.masterClock.elapsed();
            if(s<ComponentSpark>().brightness < 0.f)
                s.destroy();
        }
    }
};

struct SystemIterateAllEntities
{
    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    __declspec(noinline) auto update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        std::size_t i = 0;
        double sum = 0;

        auto view = db.unfilteredView();
        auto iter = view.begin();
        const auto end = view.end();

        while(iter != end)
        {
            auto &&e = *iter;
            for(auto j = 0; j < 10; ++j)
            {
                component<ComponentPosition>(e).position.x() = (float)++i;
                sum += *std::launder<float>(&component<ComponentPosition>(e).position.x());
            }
            ++iter;
        }
        //
        // for(auto &&e : db.unfilteredView())
        // {
        //     component<ComponentPosition>(e).position.x() = (float)++i;
        //     sum += *std::launder<float>(&component<ComponentPosition>(e).position.x());
        // }
        return std::pair(i, sum);
    }
};

struct SystemIterateFilteredEntities
{
    using ReadAccess = ComponentFilter<ComponentFireworks>;
    using WriteAccess = ComponentFilter<ComponentFireworks, ComponentPosition>;

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    __declspec(noinline) auto update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        std::size_t i = 0;
        double sum = 0;

        auto view = db.view(ReadAccess());
        auto iter = view.begin();
        const auto end = view.end();

        // x<(ComponentTypeTag<ComponentFireworks>(), rt).
        while(iter != end)
        {
            auto &&e = *iter;
            // e(ComponentTypeTag<ComponentFireworks>()).
            for(auto j = 0; j < 10; ++j)
            {
                USAGI_COMPONENT(e, ComponentFireworks).time_to_explode = (float)++i;
                sum += *std::launder<float>(&component<ComponentFireworks>(e).time_to_explode);
            }
            ++iter;
        }

        return std::pair(i, sum);
    }
};


void testaccess()
{
    Database db;

    SystemIterateFilteredEntities s6;
    int rt = 0;
    s6.update(rt, EntityDatabaseExternalAccess<
        Database,
        ComponentAccessSystemAttribute<SystemIterateFilteredEntities>
    >(&db));
}


const auto num = 50000000;

std::vector<ComponentPosition> vec0(num);
std::vector<ComponentFireworks> vec1(num);

__declspec(noinline)
auto test0(int num)
{
    std::size_t i = 0;
    double sum = 0;

    auto iter = vec0.begin();
    const auto end = vec0.end();

    while(iter != end)
    {
        auto &&e = *iter;
        for(auto j = 0; j < 10; ++j)
        {
            e.position.x() = (float)++i;
            sum += *std::launder<float>(&e.position.x());
        }
        ++iter;
    }
    // for(auto && e : vec0)
    // {
    //     e.position.x() = (float)++i;
    //     sum += *std::launder<float>(&e.position.x());
    // }

    return std::pair(i, sum);
}

__declspec(noinline)
auto test1(int num)
{
    std::size_t i = 0;
    double sum = 0;

    auto iter = vec1.begin();
    const auto end = vec1.end();

    while(iter != end)
    {
        auto &&e = *iter;
        for(auto j = 0; j < 10; ++j)
        {
            e.time_to_explode = (float)++i;
            sum += *std::launder<float>(&e.time_to_explode);
        }
        ++iter;
    }

    // for(auto && e : vec1)
    // {
    //     e.time_to_explode = (float)++i;
    //     sum += *std::launder<float>(&e.time_to_explode);
    // }
    return std::pair(i, sum);
}

__declspec(noinline)
auto init(Database &db, int num)
{
    db.create(ArchetypeFireworks(), num);
    ArchetypeSpark x;
    // db.create(x, num);
}

__declspec(noinline)
auto test2(Database &db, int num)
{
    SystemIterateAllEntities s5;
    int rt = 0;
    return s5.update(rt, EntityDatabaseExternalAccess<
        Database,
        ComponentAccessAllowAll
    >(&db));
}

__declspec(noinline)
auto test3(Database &db, int num)
{
    SystemIterateFilteredEntities s6;
    int rt = 0;
    return s6.update(rt, EntityDatabaseExternalAccess<
        Database,
        ComponentAccessAllowAll
    >(&db));
}

int usagi_main(const std::vector<std::string> &args)
{

    Database db;

    init(db, num);
    for(auto i = 0; i < 1; ++i)
    {
        {
            PrintTimer p;
            auto [i, sum] = test0(num);
            LOG(info, "vec ComponentPosition: i={}, sum={}", i, sum);
        }
        {
            PrintTimer p;
            auto [i, sum] = test1(num);
            LOG(info, "vec ComponentFireworks: i={}, sum={}", i, sum);
        }
        {
            PrintTimer p;
            auto [i, sum] = test2(db, num);
            LOG(info, "ecs unfilteredView(ComponentFireworks): i={}, sum={}", i, sum);
        }
        {
            PrintTimer p;
            auto [i, sum] =  test3(db, num);
            LOG(info, "ecs view<ComponentFireworks>: i={}, sum={}", i, sum);
        }
    }

    // s0.update(rt, db.createAccess<PermissionValidatorAllowReadWrite>());
    // s0.update(rt, db.createAccess<PermissionValidatorAllowReadWrite>());



    // LOG(info, "====================");


    // db.create(x, 33);

    // // Build the execution graph
    // auto task = exec
    //     .run<FireworksSpawnSystem>()
    //     .then<FireworksExplodeSystem>()
    //     .then<SparkFadingSystem, PhysicsSystem>()
    //     .then<SpriteRenderingSystem>();

    // SystemFireworksSpawn s0;
    // SystemFireworksExplode s1;
    // SystemSparkFading s2;
    // SystemPhysics s3;
    // SystemSpriteRendering s4;



    return 0;
}
*/
