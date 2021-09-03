// Basic Swapchain Creation Sample based on Vulkan. It demonstrates how to
// create a swapchain by creating a window entity.

#define _CRT_SECURE_NO_WARNINGS

// #define NDEBUG

#ifdef _DEBUG
#pragma comment(lib, "fmtd.lib")
#else
#pragma comment(lib, "fmt.lib")
#endif

#include <fmt/ostream.h>

#include <Usagi/Modules/Common/Executive/AppHost.hpp>
#include <Usagi/Modules/Common/Executive/SystemTaskList.hpp>
#include <Usagi/Modules/Common/Math/ComponentRegion2D.hpp>
#include <Usagi/Modules/Services/Input/InputEventQueue.hpp>
#include <Usagi/Modules/Services/Input/SystemInputEventPump.hpp>
#include <Usagi/Modules/Services/Windowing/ComponentNativeWindow.hpp>
#include <Usagi/Modules/Services/Windowing/ServiceNativeWindowManager.hpp>
#include <Usagi/Modules/Services/Windowing/SystemNativeWindowCoordinator.hpp>

#include <Usagi/Modules/Platforms/WinCommon/Input/InputEventSourceWin32RawInput.hpp>
#include <Usagi/Modules/Platforms/WinCommon/Windowing/NativeWindowManagerWin32.hpp>

#include "SystemInitGraphicsService.hpp"

using namespace usagi;

struct Services
    : ServiceInputSource
    , ServiceNativeWindowManager
    , ServiceStateTransitionGraph
    , ServiceHardwareGraphics
{
    Services()
        : ServiceInputSource(Tag<InputEventSourceWin32RawInput>())
        // todo: use templatized type to help devirtualization (but how to change service provider during runtime?)
        , ServiceNativeWindowManager(Tag<NativeWindowManagerWin32>())
    {
    }
};

Archetype<
    ComponentNativeWindow,
    ComponentRegion2D
> gArchetypeWindow;

using TaskList = SystemTaskList<
    SystemNativeWindowCoordinator,
    SystemInitGraphicsService
>;
TaskList gTaskList;

using App = AppHost<Services, TaskList>;

int main(int argc, char *argv[])
{
    std::filesystem::remove_all("demo_init_gfx");

    App app { "demo_init_gfx" };
    auto &db = app.database_world();
    InputEventQueue input_event_queue;
    SystemInputEventPump pump;

    if(std::ranges::distance(db.create_access<ComponentAccessReadOnly>()
        .view<ComponentNativeWindow>()) == 0)
    {
        auto &c_wnd = gArchetypeWindow.component<ComponentNativeWindow>();
        c_wnd.identifier = "usagi";
        c_wnd.on_close = ComponentNativeWindow::NOTIFY_EXIT;
        auto &c_region = gArchetypeWindow.component<ComponentRegion2D>();
        c_region.size = { 1280, 720 };
        c_region.position = { 100, 100 };
        db.insert(gArchetypeWindow);
    }

    // auto &wnd_mgr = USAGI_SERVICE(gServices, ServiceNativeWindowManager);

    auto &tg = USAGI_SERVICE(app.services(), ServiceStateTransitionGraph);
    while(!tg.should_exit)
    {
        auto sys_access = input_event_queue.create_access<
            ComponentAccessSystemAttribute<SystemInputEventPump>
        >();

        pump.update(app.services(), sys_access);

        auto view_access = input_event_queue.create_access<
            ComponentAccessAllowAll
        >();

        for(auto &&e : view_access.view<ComponentInputEvent>())
        {
            auto &msg = e.component<ComponentInputEvent>();
            [[maybe_unused]]
            auto &time = e.component<ComponentTimestamp>();

            if(msg.axis == InputAxis::Q && msg.pressed())
            {
                tg.should_exit = true;
            }

            e.destroy();
        }

        input_event_queue.reclaim_pages();

        app.update();
    }
}
