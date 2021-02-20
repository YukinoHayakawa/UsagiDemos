// Window creation and keyboard input demo

#ifdef _DEBUG
#pragma comment(lib, "fmtd.lib")
#else
#pragma comment(lib, "fmt.lib")
#endif

#include <fmt/ostream.h>

#include <Usagi/Module/Platform/WinCommon/Input/InputEventSourceWin32RawInput.hpp>
#include <Usagi/Module/Service/Input/InputEventQueue.hpp>
#include <Usagi/Module/Service/Input/SystemInputEventPump.hpp>

using namespace usagi;

struct Services
    : ServiceInputSource
{
    Services()
        : ServiceInputSource(std::make_unique<InputEventSourceWin32RawInput>())
    {
    }
} gServices;

int main(int argc, char *argv[])
{
    InputEventQueue input_event_queue;
    SystemInputEventPump pump;

    while(true)
    {
        auto sys_access = input_event_queue.create_access<
            ComponentAccessSystemAttribute<SystemInputEventPump>
        >();

        pump.update(gServices, sys_access);

        auto view_access = input_event_queue.create_access<
            ComponentAccessAllowAll
        >();

        for(auto &&e : view_access.view<ComponentInputEvent>())
        {
            auto &msg = e.component<ComponentInputEvent>();
            fmt::print(
                "axis={}, "
                "abs={{{},{}}}, rel={{{},{}}}, "
                "pressed={}, released={}\n",
                to_string(msg.axis),
                msg.absolute.x(),
                msg.absolute.y(),
                msg.relative.x(),
                msg.relative.y(),
                msg.pressed(),
                msg.released()
            );
            e.destroy();
        }

        input_event_queue.reclaim_pages();
    }
}
