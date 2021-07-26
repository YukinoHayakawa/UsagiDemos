﻿// Window creation and keyboard input demo

#define _CRT_SECURE_NO_WARNINGS

// #define NDEBUG

#ifdef _DEBUG
#pragma comment(lib, "fmtd.lib")
#else
#pragma comment(lib, "fmt.lib")
#endif

#include <fmt/ostream.h>
#include <Usagi/Module/Common/Executive/AppHost.hpp>

#include <Usagi/Module/Common/Executive/SystemTaskList.hpp>
#include <Usagi/Module/Common/Math/ComponentRegion2D.hpp>
#include <Usagi/Module/Service/Input/InputEventQueue.hpp>
#include <Usagi/Module/Service/Input/SystemInputEventPump.hpp>
#include <Usagi/Module/Service/Windowing/ComponentNativeWindow.hpp>
#include <Usagi/Module/Service/Windowing/ServiceNativeWindowManager.hpp>
#include <Usagi/Module/Service/Windowing/SystemNativeWindowCoordinator.hpp>

#include <Usagi/Module/Platform/WinCommon/Input/InputEventSourceWin32RawInput.hpp>
#include <Usagi/Module/Platform/WinCommon/Windowing/NativeWindowManagerWin32.hpp>

using namespace usagi;

struct Services
    : ServiceInputSource
    , ServiceNativeWindowManager
    , ServiceStateTransitionGraph
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
    SystemNativeWindowCoordinator
>;
TaskList gTaskList;

using App = AppHost<Services, TaskList>;

int main(int argc, char *argv[])
{
    App app { "demo_window_input" };
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
            auto &time = e.component<ComponentTimestamp>();

            fmt::print(
                "time={}, "
                "axis={}, "
                "abs={{{},{}}}, rel={{{},{}}}, "
                "pressed={}, released={}\n",
                time.seconds,
                to_string(msg.axis),
                msg.absolute.x(), msg.absolute.y(),
                msg.relative.x(), msg.relative.y(),
                msg.pressed(), msg.released()
            );

            if(msg.axis == InputAxis::A && msg.pressed())
            {
                union name
                {
                    char n[8];
                    std::uint64_t i = 0;
                };
                static name n;
                auto &c_region = gArchetypeWindow.component<ComponentRegion2D>();
                c_region.size = { 1280, 720 };
                c_region.position += Vector2f { 100, 100 };
                auto &c_wnd = gArchetypeWindow.component<ComponentNativeWindow>();
                ++n.i;
                c_wnd.identifier = n.n;
                c_wnd.on_close = ComponentNativeWindow::DESTROY_ENTITY;
                db.insert(gArchetypeWindow);
            }

            e.destroy();
        }

        input_event_queue.reclaim_pages();

        app.update();
    }
}
