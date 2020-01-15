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
#include <Usagi/Utility/PrintTimer.hpp>
#include <Usagi/Experimental/v2/Game/_detail/ComponentAccessSystemAttribute.hpp>

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
    float size;
};
static_assert(Component<ComponentSprite>);

using ArchetypeFireworks = Archetype<
    ComponentFireworks,
    ComponentPosition,
    ComponentPhysics,
    ComponentSprite
>;

using ArchetypeSpark = Archetype<
    ComponentSpark,
    ComponentPosition,
    ComponentPhysics,
    ComponentSprite
>;

// todo infer components from systems
using Database = EntityDatabase<
    16,
    ComponentFireworks,
    ComponentSpark,
    ComponentPosition,
    ComponentPhysics,
    ComponentSprite
>;

struct SystemFireworksSpawn
{
    using WriteAccess = ArchetypeFireworks::ComponentFilterT;

    std::mt19937 gen { std::random_device()() };
    std::uniform_real_distribution<float> dis { 1.f, 2.f };
    Clock timer;
    float wait_time_sec = dis(gen);

    ArchetypeFireworks fireworks;
    ArchetypeFireworks fireworks2;

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&svc, EntityDatabaseAccess &&db)
    {
        // if(timer.elapsed() > wait_time_sec)
        for(auto i = 0; i < 17; ++i)
        {
            fireworks.val<ComponentFireworks>().num_sparks = 10;
            fireworks.val<ComponentFireworks>().time_to_explode = 2;
            fireworks.val<ComponentPosition>().position = { 1280 / 2, 0 };
            fireworks.val<ComponentPhysics>().velocity = { 0, 0 };
            fireworks.val<ComponentPhysics>().acceleration = { 0, 0 };
            fireworks.val<ComponentSprite>().size = 5;
            EntityId e;
            EntityId es[3];
            for(auto j = 0; j < 17; ++j)
            {
                e = db.create(fireworks, 3, es);
                for(auto &ei : es)
                {
                    fmt::print("{} {}\n", ei.id, ei.page_id);
                }
            }
            for(auto j = 0; j < 3; ++j)
            {
                e = db.create(fireworks2, 17);
                fmt::print("{} {}\n", e.id, e.page_id);
            }
            //
            //
            // e = db.create(fireworks);
            //
            // auto &c_f = USAGI_COMPONENT(e, ComponentFireworks);
            // c_f.num_sparks = 10;
            // c_f.time_to_explode = 2;
            //
            // auto &c_pos = USAGI_COMPONENT(e, ComponentPosition);
            // c_pos.position = { 1280 / 2, 0 };
            //
            // auto &c_phy = USAGI_COMPONENT(e, ComponentPhysics);
            // c_phy.velocity = { 0, 0, 0 };
            // c_phy.acceleration = { 0, 0, 0 };
            //
            // auto &c_s = USAGI_COMPONENT(e, ComponentSprite);
            // c_s.size = 5;


            timer.reset();
            wait_time_sec = dis(gen);
        }
    }
};

void t()
{
    SystemFireworksSpawn s;
    Database db;
    int rt = 0;
    s.update(rt, EntityDatabaseAccessExternal<
        Database,
        ComponentAccessSystemAttribute<SystemFireworksSpawn>
    >(&db));
}

int usagi_main(const std::vector<std::string> &args)
{
    t();

    return 0;
}

/*
struct SystemFireworksExplode
{
    using WriteAccess = ComponentFilter<
        ComponentFireworks,
        ComponentSpark,
        ComponentPosition,
        ComponentPhysics,
        ComponentSprite
    >;

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        auto fireworks_view = db.view<decltype(gFireworks)::component_mask_t>;

        for(auto &&v : db.view(ComponentFilter<ComponentFireworks>()))
        {
            auto &f_c = USAGI_COMPONENT(v, ComponentFireworks);

            f_c.time_to_explode -= rt.masterClock.elapsed();

            if(f_c.time_to_explode < 0.f)
            {
                for(auto i = 0; i < f_c.num_sparks; ++i)
                {
                    auto e = db.create();
                    e<ComponentSpark>() = ...;
                    e<ComponentPosition>() = f<ComponentPosition>();
                    e<ComponentPhysics>() = ...;
                    e<ComponentSprite>() = ...;
                }
                f.destroy();
            }
        }
    }
};

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

struct SystemPhysics
{
    using WriteAccess = ComponentFilter<ComponentPosition, ComponentPhysics>;

    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        auto view = db.view<write_access>;
        auto dt = rt.masterClock.elapsed();
        for(auto &&p : view)
        {
            auto &pc = p<ComponentPhysics>();
            p<ComponentPosition>().position += pc.velocity * dt;
            pc.velocity += pc.acceleration * dt;
        }
    }
};

struct SystemSpriteRendering
{
    using ReadAccess = ComponentFilter<ComponentPosition, ComponentSprite>;

    // EntityDB is a concept that covers all instantiations of the
    // database template
    // EntityDatabaseAccess adds layer of permission checking. if the system
    // does not have access to some components, the access is denied.
    template <typename RuntimeServiceProvider, typename EntityDatabaseAccess>
    void update(RuntimeServiceProvider &&rt, EntityDatabaseAccess &&db)
    {
        auto fb = rt.resourse<RawFramebuffer>(
            "res://io/window/0/framebuffer"
        );

        auto sprites = db.view<read_access>;
        for(auto &&s : sprites)
        {
            fb.drawPoint(
                s<ComponentPosition>().position,
                s<ComponentSprite>().size
            );
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
