#pragma once

#include "Type.hpp"
#include "Runtime.hpp"
#include "Service_graphics_gdi.hpp"

struct System_sprite_render
{
    using ReadAccess = ComponentFilter<
        ComponentPosition,
        ComponentSprite,
        ComponentColor
    >;

    // EntityDB is a concept that covers all instantiations of the
    // database template
    // EntityDatabaseAccess adds layer of permission checking. if the system
    // does not have access to some components, the access is denied.
    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        // https://stackoverflow.com/questions/1937163/drawing-in-a-win32-console-on-c

        HDC &cdc = USAGI_SERVICE(rt, Service_graphics_gdi).cdc;

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
        //
        // // draw stats
        // SelectObject(cdc, GetStockObject(ANSI_FIXED_FONT));
        // TextOutW(cdc, 10, 10, L"Sample ANSI_FIXED_FONT text", 64);
    }
};
