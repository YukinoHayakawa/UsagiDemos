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

        HDC &cdc = USAGI_SERVICE(rt, Service_graphics_gdi).cdc;
        Bitmap &bitmap = USAGI_SERVICE(rt, Service_graphics_gdi).bitmap;

        SelectObject(cdc, GetStockObject(WHITE_PEN));
        MoveToEx(cdc, 0, 1080, nullptr);
        LineTo(cdc, 1920, 1080);
        LineTo(cdc, 1920, 0);

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
            bitmap.fill_rect(
                (int)pos.position.x(),
                (int)pos.position.y(),
                sprite.size,
                sprite.size,
                rgb
            );
        }
        //
        // // draw stats
        // SelectObject(cdc, GetStockObject(ANSI_FIXED_FONT));
        // TextOutW(cdc, 10, 10, L"Sample ANSI_FIXED_FONT text", 64);
    }
};
