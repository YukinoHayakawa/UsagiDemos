#pragma once

#include "Type.hpp"
#include "Runtime.hpp"
#include "Service_graphics_gdi.hpp"

struct System_background_render
{
    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        Bitmap &bitmap = USAGI_SERVICE(rt, Service_graphics_gdi).bitmap;
        bitmap.fill_rect(0, 0, 1920, 1080, 0);
    }
};

