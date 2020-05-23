#pragma once

#include <execution>

#include <Usagi/Runtime/Service.hpp>

#include "Type.hpp"
#include "Service_graphics_gdi.hpp"

struct System_sprite_render
{
    using ReadAccess = ComponentFilter<
        ComponentSprite,
        ComponentPosition,
        ComponentColor
    >;

    // EntityDB is a concept that covers all instantiations of the
    // database template
    // EntityDatabaseAccess adds layer of permission checking. if the system
    // does not have access to some components, the access is denied.
    template <typename RuntimeServices, typename EntityDatabaseAccess>
    std::size_t update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        HDC &cdc = USAGI_SERVICE(rt, Service_graphics_gdi).cdc;
        (void)cdc;
        Bitmap &bitmap = USAGI_SERVICE(rt, Service_graphics_gdi).bitmap;

        std::atomic<std::size_t> count = 0;
        std::for_each(std::execution::par, db.begin(), db.end(), [&](auto &&p) {
            for(auto &&e : db.page_view(p, ReadAccess()))
            {
                auto &pos = USAGI_COMPONENT(e, ComponentPosition);
                auto &sprite = USAGI_COMPONENT(e, ComponentSprite);
                auto &color = USAGI_COMPONENT(e, ComponentColor);

                const auto rgb = RGB(
                    static_cast<int>(color.rgb.x()),
                    static_cast<int>(color.rgb.y()),
                    static_cast<int>(color.rgb.z())
                );
                bitmap.fill_rect(
                    static_cast<int>(pos.position.x()),
                    static_cast<int>(pos.position.y()),
                    sprite.size,
                    sprite.size,
                    rgb
                );
                ++count; // VERY EXPENSIVE
            }
        });

        return count.load();
    }
};
