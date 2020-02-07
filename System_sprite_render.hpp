#pragma once

#include <Usagi/Extensions/RtWin32/Win32.hpp>

#include "Type.hpp"

struct System_sprite_render
{
    using ReadAccess = ComponentFilter<
        ComponentPosition,
        ComponentSprite,
        ComponentColor
    >;

    System_sprite_render()
    {
        HWND console_handle = GetConsoleWindow();

        SetWindowPos(console_handle, nullptr, 0, 0, 2000, 1200, SWP_NOMOVE);
    }

    // EntityDB is a concept that covers all instantiations of the
    // database template
    // EntityDatabaseAccess adds layer of permission checking. if the system
    // does not have access to some components, the access is denied.
    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
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

            const auto rgb = RGB(
                (int)color.rgb.x(),
                (int)color.rgb.y(),
                (int)color.rgb.z()
            );
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
