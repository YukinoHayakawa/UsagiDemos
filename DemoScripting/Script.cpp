// #include "Database.hpp"

using namespace usagi;

Archetype<
    TestComponent1
> gArchetype1;

Archetype<
    TestComponent2
> gArchetype2;

enum ResumeCondition : std::uint8_t
{
    NEVER,
    USER_INPUT,
    NEXT_FRAME,
};

std::pair<std::uint64_t, ResumeCondition>
script(std::uint64_t entry, auto &&db) {
    switch(entry)
    {
        case 0:
        {
            gArchetype1(C<TestComponent1>()).num = 123456;
            db.insert(gArchetype1);
            gArchetype1(C<TestComponent1>()).num = 567890;
            db.insert(gArchetype1);
            return { 1, NEXT_FRAME };
        }
        case 1:
        {
            for(auto &&e : db.view(C<TestComponent1>()))
                if(component<TestComponent1>(e).num == 123456)
                    e.destroy();
            gArchetype2(C<TestComponent2>()).flt = 123.456f;
            return { 2, NEXT_FRAME };
        }
        case 2:
        {
            for(auto &&e : db.view(C<TestComponent1>()))
                printf("%llu\n", component<TestComponent1>(e).num);
            for(auto &&e : db.view(C<TestComponent2>()))
                printf("%f\n", component<TestComponent2>(e).flt);
        }
        default: return { -1, NEVER };
    }
}
//
// std::pair<std::uint64_t, ResumeCondition>
// script_main(std::uint64_t entry /* insert db type */)
// {
//     return script(entry, db);
// }
