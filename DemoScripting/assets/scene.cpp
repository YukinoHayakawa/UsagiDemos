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
    puts("1");
    switch(entry)
    {
        case 0:
        {
            puts("2");
            gArchetype1(C<TestComponent1>()).num = 123456;
            puts("3");
            db.insert(gArchetype1);
            puts("4");
            gArchetype1(C<TestComponent1>()).num = 567890;
            puts("5");
            db.insert(gArchetype1);
            puts("6");
            return { 1, NEXT_FRAME };
        }
        case 1:
        {
            for(auto &&e : db.view(C<TestComponent1>()))
                if(component<TestComponent1>(e).num == 123456)
                    e.destroy();
            gArchetype2(C<TestComponent2>()).flt = 123.456f;
            db.insert(gArchetype2);
            return { 2, NEXT_FRAME };
        }
        case 2:
        {
            puts("TestComponent1:");
            for(auto &&e : db.view(C<TestComponent1>()))
                printf("%llu\n", component<TestComponent1>(e).num);
            puts("TestComponent2:");
            for(auto &&e : db.view(C<TestComponent2>()))
                printf("%f\n", component<TestComponent2>(e).flt);
        }
        default: return { -1, NEVER };
    }
}
