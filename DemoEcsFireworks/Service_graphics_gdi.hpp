#pragma once

#include <memory>
#include <algorithm>

#include <Usagi/Module/Platform/WinCommon/Win32.hpp>

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

    Pixel *buffer = nullptr;

    void fill_rect(
        const int x,
        const int y,
        const int w,
        const int h,
        const int color)
    {
        auto ii = std::min(x + w, width);
        auto jj = std::min(y + h, height);
        for(int j = std::max(0, y); j < jj; ++j)
            for(int i = std::max(0, x); i < ii; ++i)
                buffer[width * j + i].rgb = color;
    }

    void clear(int v)
    {
        memset(buffer, v, width * height * 4);
    }
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool run_game();

struct GdiGraphics
{
    HWND hwnd;
    HDC cdc;
    HBITMAP bitmap_handle;
    Bitmap bitmap;

    // https://docs.microsoft.com/en-us/windows/win32/gdi/memory-device-contexts
    // https://stackoverflow.com/questions/7502588/createcompatiblebitmap-and-createdibsection-memory-dcs
    // https://devblogs.microsoft.com/oldnewthing/20170331-00/?p=95875
    // https://stackoverflow.com/questions/1937163/drawing-in-a-win32-console-on-c

    GdiGraphics()
    {
        // https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program

        // Register the window class.
        const wchar_t CLASS_NAME[]  = L"Sample Window Class";

        WNDCLASS wc = { };

        wc.lpfnWndProc   = WindowProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;

        RegisterClass(&wc);

        // Create the window.

        const auto style = WS_OVERLAPPEDWINDOW;

        RECT rect = { };
        rect.right = 1920;
        rect.bottom = 1080;
        AdjustWindowRect(&rect, style, FALSE);

        hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            L"ECS Fireworks Demo",    // Window text
            style,            // Window style

                                            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,

            NULL,       // Parent window
            NULL,       // Menu
            wc.hInstance,  // Instance handle
            NULL        // Additional application data
        );


        ShowWindow(hwnd, true);


        HDC hdc = GetDC(hwnd);
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

        ReleaseDC(hwnd, hdc);

        SelectObject(cdc, bitmap_handle);
    }

    ~GdiGraphics()
    {
        DeleteDC(cdc);
    }

    void present()
    {
        GdiFlush();
        HDC hdc = GetDC(hwnd);
        BitBlt(hdc, 0, 0, 1920, 1080, cdc, 0, 0, SRCCOPY);
        ReleaseDC(hwnd, hdc);
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
