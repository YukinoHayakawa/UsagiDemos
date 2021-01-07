// Window creation and keyboard input demo

#ifdef _DEBUG
#pragma comment(lib, "fmtd.lib")
#else
#pragma comment(lib, "fmt.lib")
#endif

#include <fmt/ostream.h>

#include <Usagi/Module/Platform/WinCommon/Input/InputEventSourceWin32RawInput.hpp>
#include <Usagi/Module/Service/Input/InputEventVisitor.hpp>

using namespace usagi;

struct Visitor : InputEventVisitor
{
    void keyboard_event(
        const ArchetypeKeyboardEvent &archetype,
        bool pressed) override
    {
        fmt::print("key={}, pressed={}\n",
            archetype.component<ComponentKeyCode>().code,
            pressed
        );
    }

    void mouse_button_event(
        const ArchetypeMouseButtonEvent &archetype,
        bool pressed) override
    {
    }

    void mouse_position_event(
        const ArchetypeMousePositionEvent &archetype) override
    {
    }
};

int main(int argc, char *argv[])
{
    InputEventSourceWin32RawInput pump;
    Visitor visitor;

    while(true)
        pump.pump_events(visitor);
}
