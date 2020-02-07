#pragma once

#include <Usagi/Extensions/RtWin32/Win32.hpp>

namespace usagi
{
struct Bitmap
{
    int width = 1920, height = 1080;

    union Pixel
    {
        BYTE a, b, g, r;
        int rgb;
    };

    std::unique_ptr<Pixel> buffer;

    Bitmap()
    {
        buffer.reset(new Pixel[width * height]);
    }

    void fill_rect(
        const int x,
        const int y,
        const int w,
        const int h,
        const int color)
    {
        for(int i = std::max(0, x); i < std::min(x + w, width); ++i)
            for(int j = std::max(0, y); j < std::min(y + h, height); ++j)
                buffer.get()[i + width * j].rgb = color;
    }
};

struct GdiGraphics
{
    HWND console_handle;
    HDC cdc;
    HBITMAP bitmap_handle;
    Bitmap bitmap;

    // https://docs.microsoft.com/en-us/windows/win32/gdi/memory-device-contexts
    // https://stackoverflow.com/questions/7502588/createcompatiblebitmap-and-createdibsection-memory-dcs
    // https://devblogs.microsoft.com/oldnewthing/20170331-00/?p=95875
    // https://stackoverflow.com/questions/1937163/drawing-in-a-win32-console-on-c

    GdiGraphics()
    {

        console_handle = GetConsoleWindow();
        SetWindowPos(console_handle, nullptr, 0, 0, 2000, 1200, SWP_NOMOVE);

        HDC hdc = GetDC(console_handle);
        cdc = CreateCompatibleDC(hdc);

        BITMAPINFO bi;
        ZeroMemory(&bi, sizeof(BITMAPINFO));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = 1920;
        bi.bmiHeader.biHeight = 1080;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;

        bitmap_handle = CreateDIBSection(
            cdc, &bi, DIB_RGB_COLORS,
            (LPVOID*)&bitmap.buffer, NULL, 0);

        ReleaseDC(console_handle, hdc);

        SelectObject(cdc, bitmap_handle);
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
