#pragma once

#include <Usagi/Extensions/RtWin32/Win32.hpp>

namespace usagi
{
struct GdiGraphics
{
    HWND console_handle;
    HDC cdc;
    HBITMAP buffer;

    // https://docs.microsoft.com/en-us/windows/win32/gdi/memory-device-contexts
    // https://stackoverflow.com/questions/7502588/createcompatiblebitmap-and-createdibsection-memory-dcs
    // https://devblogs.microsoft.com/oldnewthing/20170331-00/?p=95875

    GdiGraphics()
    {
        console_handle = GetConsoleWindow();
        SetWindowPos(console_handle, nullptr, 0, 0, 2000, 1200, SWP_NOMOVE);

        HDC hdc = GetDC(console_handle);
        cdc = CreateCompatibleDC(hdc);
        buffer = CreateCompatibleBitmap(hdc, 1920, 1080);
        ReleaseDC(console_handle, hdc);

        SelectObject(cdc, buffer);
    }

    ~GdiGraphics()
    {
        DeleteDC(cdc);
    }

    void present()
    {
        GdiFlush();
        HDC hdc = GetDC(console_handle);
        BitBlt(hdc, 0, 0, 1920, 1080, cdc, 0, 0, SRCCOPY);
        ReleaseDC(console_handle, hdc);
    }
};

struct Service_graphics_gdi
{
    using ServiceType = GdiGraphics;

    ServiceType & get_service()
    {
        return mGdi;
    }

private:
    GdiGraphics mGdi; // service implementation
};
}
