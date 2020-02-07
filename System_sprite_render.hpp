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

    HWND console_handle;
    HDC cdc;
    HBITMAP buffer;

    // https://docs.microsoft.com/en-us/windows/win32/gdi/memory-device-contexts
    // https://stackoverflow.com/questions/7502588/createcompatiblebitmap-and-createdibsection-memory-dcs
    // https://devblogs.microsoft.com/oldnewthing/20170331-00/?p=95875

    System_sprite_render()
    {
        console_handle = GetConsoleWindow();
        SetWindowPos(console_handle, nullptr, 0, 0, 2000, 1200, SWP_NOMOVE);

        HDC hdc = GetDC(console_handle);
        cdc = CreateCompatibleDC(hdc);
        buffer = CreateCompatibleBitmap(hdc, 1920, 1080);
        ReleaseDC(console_handle, hdc);

        SelectObject(cdc, buffer);
    }

    ~System_sprite_render()
    {
        DeleteDC(cdc);
    }

    // EntityDB is a concept that covers all instantiations of the
    // database template
    // EntityDatabaseAccess adds layer of permission checking. if the system
    // does not have access to some components, the access is denied.
    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        // https://stackoverflow.com/questions/1937163/drawing-in-a-win32-console-on-c

        SelectObject(cdc, GetStockObject(WHITE_PEN));
        MoveToEx(cdc, 0, 1080, nullptr);
        LineTo(cdc, 1920, 1080);
        LineTo(cdc, 1920, 0);

        SelectObject(cdc, GetStockObject(BLACK_BRUSH));
        Rectangle(cdc, 0, 0, 1920, 1080);

        SelectObject(cdc, GetStockObject(DC_PEN));
        SelectObject(cdc, GetStockObject(DC_BRUSH));
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
            SetDCPenColor(cdc, rgb);
            SetDCBrushColor(cdc, rgb);
            Rectangle(cdc,
                (int)pos.position.x(),
                1080 - (int)pos.position.y(),
                (int)pos.position.x()+ sprite.size,
                1080 - ((int)pos.position.y() + sprite.size)
            );
        }

        GdiFlush();
        HDC hdc = GetDC(console_handle);
        BitBlt(hdc, 0, 0, 1920, 1080, cdc, 0, 0, SRCCOPY);
        ReleaseDC(console_handle, hdc);
    }
};
