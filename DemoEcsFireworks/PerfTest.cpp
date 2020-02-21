
/*


struct SystemSparkFading
{
using WriteAccess = ComponentFilter<ComponentSpark>;

template <typename RuntimeServices, typename EntityDatabaseAccess>
void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
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
template <typename RuntimeServices, typename EntityDatabaseAccess>
__declspec(noinline) auto update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
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

template <typename RuntimeServices, typename EntityDatabaseAccess>
__declspec(noinline) auto update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
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
