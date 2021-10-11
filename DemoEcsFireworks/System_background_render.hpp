#pragma once

#include <Usagi/Runtime/Service/Service.hpp>

#include "Service_graphics_gdi.hpp"

struct System_background_render
{
    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        Bitmap &bitmap = USAGI_SERVICE(rt, Service_graphics_gdi).bitmap;
        bitmap.clear(0);
    }
};

