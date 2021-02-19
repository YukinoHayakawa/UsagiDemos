#pragma once

#include <Usagi/Entity/detail/ComponentFilter.hpp>
#include <Usagi/Runtime/Service.hpp>

#include "Service_master_clock.hpp"
#include "Service_graphics_gdi.hpp"
#include "Service_stat.hpp"

#include <psapi.h>

namespace usagi
{
struct System_draw_stat
{
    using WriteAccess = ComponentFilter<>;
    using ReadAccess = ComponentFilter<>;
    wchar_t buf[64] = { };
    PROCESS_MEMORY_COUNTERS_EX mem {};
    HFONT font;
    const int font_height = 25;

    System_draw_stat()
    {
        // http://forums.codeguru.com/showthread.php?481523-Changing-Font-size
        LOGFONT logFont;
        GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(logFont), &logFont);
        logFont.lfHeight = font_height;
        logFont.lfWidth = 0;
        logFont.lfPitchAndFamily = DEFAULT_PITCH + FF_MODERN;
        font = CreateFontIndirect(&logFont);
    }

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        const auto dt =
            USAGI_SERVICE(rt, Service_master_clock).last_frame_time();
        auto &gdi = USAGI_SERVICE(rt, Service_graphics_gdi);
        auto &stat = USAGI_SERVICE(rt, Service_stat);

        const auto num_entries = 15;
        auto y = 1070 - (num_entries + 1) * font_height;

        GetProcessMemoryInfo(
            GetCurrentProcess(),
            reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&mem),
            sizeof(PROCESS_MEMORY_COUNTERS_EX)
        );

        // https://docs.microsoft.com/en-us/windows/win32/gdi/using-a-stock-font-to-draw-text
        SelectObject(gdi.cdc, font);

#define TEXT_OUT(fmt, data) \
    TextOutW( \
        gdi.cdc, 10, y += font_height, buf, \
        swprintf(buf, 64, fmt, data)) \
/**/
        TEXT_OUT(L"Num Sprites          : %llu", stat.sprite_count);
        TEXT_OUT(L"Frame Time           : %.3lfms", dt * 1000.0);
        TEXT_OUT(L"Time Per 1k Sprites  : %.3lfms",
            dt * 1000.0 / stat.sprite_count * 1000.0);
        TEXT_OUT(L"Message Loop         : %.3lfms", stat.time_input * 1000.0);
        TEXT_OUT(L"Fireworks Spawn      : %.3lfms", stat.time_spawn * 1000.0);
        TEXT_OUT(L"Fireworks Explode    : %.3lfms", stat.time_explode * 1000.0);
        TEXT_OUT(L"Spark Fade           : %.3lfms", stat.time_fade * 1000.0);
        TEXT_OUT(L"Physics              : %.3lfms", stat.time_physics * 1000.0);
        TEXT_OUT(L"Oob Remove           : %.3lfms", stat.time_remove * 1000.0);
        TEXT_OUT(L"Buffer Clear         : %.3lfms", stat.time_clear * 1000.0);
        TEXT_OUT(L"Sprite Render        : %.3lfms", stat.time_render * 1000.0);
        TEXT_OUT(L"Stat Render          : %.3lfms", stat.time_stat * 1000.0);
        TEXT_OUT(L"Frame Present        : %.3lfms", stat.time_present * 1000.0);
        TEXT_OUT(L"Num Page Faults      : %lu", mem.PageFaultCount);
        TEXT_OUT(L"Private Mem Usage    : %llu bytes", mem.PrivateUsage);
    }
};
}
