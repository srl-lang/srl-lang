#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_gfx.hpp"
#include "vm.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

namespace srl {

#ifdef _WIN32
static HWND gfxHwnd = NULL;
static HDC gfxHdcWindow = NULL;
static HDC gfxHdcMem = NULL;
static HBITMAP gfxHbmMem = NULL;
static HBITMAP gfxHbmOld = NULL;
static int winWidth = 800;
static int winHeight = 600;
static bool windowIsOpen = false;

static COLORREF parseColor(const std::string& col) {
    if (col == "red") return RGB(255, 0, 0);
    if (col == "green") return RGB(0, 255, 0);
    if (col == "blue") return RGB(0, 0, 255);
    if (col == "yellow") return RGB(255, 255, 0);
    if (col == "cyan") return RGB(0, 255, 255);
    if (col == "magenta") return RGB(255, 0, 255);
    if (col == "white") return RGB(255, 255, 255);
    if (col == "black") return RGB(0, 0, 0);
    if (col == "gray" || col == "grey") return RGB(128, 128, 128);
    if (col == "darkgray") return RGB(40, 40, 40);
    if (col == "orange") return RGB(255, 165, 0);
    return RGB(255, 255, 255);
}

static LRESULT CALLBACK GfxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            windowIsOpen = false;
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}
#endif

void GFX::registerNativeFunctions(VM& vm) {
    // gfx_window_create(title, width, height)
    vm.defineNative("gfx_window_create", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        std::string title = (argCount > 0 && args[0].isString()) ? args[0].asString() : "SRL Graphics Window";
        int w = (argCount > 1 && args[1].isNumber()) ? static_cast<int>(args[1].asNumber()) : 800;
        int h = (argCount > 2 && args[2].isNumber()) ? static_cast<int>(args[2].asNumber()) : 600;

        winWidth = w;
        winHeight = h;

        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASSA wc = {0};
        wc.lpfnWndProc = GfxWndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = "SRL_GFX_CLASS";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

        RegisterClassA(&wc);

        RECT rect = {0, 0, w, h};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        gfxHwnd = CreateWindowA("SRL_GFX_CLASS", title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                                NULL, NULL, hInstance, NULL);


        if (gfxHwnd) {
            gfxHdcWindow = GetDC(gfxHwnd);
            gfxHdcMem = CreateCompatibleDC(gfxHdcWindow);
            gfxHbmMem = CreateCompatibleBitmap(gfxHdcWindow, w, h);
            gfxHbmOld = (HBITMAP)SelectObject(gfxHdcMem, gfxHbmMem);
            windowIsOpen = true;
            ShowWindow(gfxHwnd, SW_SHOW);
            UpdateWindow(gfxHwnd);
            return Value(true);
        }
#endif
        return Value(false);
    });

    // gfx_clear(color_name)
    vm.defineNative("gfx_clear", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (gfxHdcMem) {
            std::string colName = (argCount > 0 && args[0].isString()) ? args[0].asString() : "black";
            COLORREF c = parseColor(colName);
            HBRUSH brush = CreateSolidBrush(c);
            RECT r = {0, 0, winWidth, winHeight};
            FillRect(gfxHdcMem, &r, brush);
            DeleteObject(brush);
        }
#endif
        return Value();
    });

    // gfx_draw_rect(x, y, w, h, color)
    vm.defineNative("gfx_draw_rect", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (gfxHdcMem && argCount >= 5) {
            int x = static_cast<int>(args[0].asNumber());
            int y = static_cast<int>(args[1].asNumber());
            int w = static_cast<int>(args[2].asNumber());
            int h = static_cast<int>(args[3].asNumber());
            COLORREF col = parseColor(args[4].asString());

            HPEN pen = CreatePen(PS_SOLID, 1, col);
            HPEN oldPen = (HPEN)SelectObject(gfxHdcMem, pen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(gfxHdcMem, GetStockObject(NULL_BRUSH));

            Rectangle(gfxHdcMem, x, y, x + w, y + h);

            SelectObject(gfxHdcMem, oldPen);
            SelectObject(gfxHdcMem, oldBrush);
            DeleteObject(pen);
        }
#endif
        return Value();
    });

    // gfx_fill_rect(x, y, w, h, color)
    vm.defineNative("gfx_fill_rect", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (gfxHdcMem && argCount >= 5) {
            int x = static_cast<int>(args[0].asNumber());
            int y = static_cast<int>(args[1].asNumber());
            int w = static_cast<int>(args[2].asNumber());
            int h = static_cast<int>(args[3].asNumber());
            COLORREF col = parseColor(args[4].asString());

            HBRUSH brush = CreateSolidBrush(col);
            RECT r = {x, y, x + w, y + h};
            FillRect(gfxHdcMem, &r, brush);
            DeleteObject(brush);
        }
#endif
        return Value();
    });

    // gfx_draw_line(x1, y1, x2, y2, color)
    vm.defineNative("gfx_draw_line", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (gfxHdcMem && argCount >= 5) {
            int x1 = static_cast<int>(args[0].asNumber());
            int y1 = static_cast<int>(args[1].asNumber());
            int x2 = static_cast<int>(args[2].asNumber());
            int y2 = static_cast<int>(args[3].asNumber());
            COLORREF col = parseColor(args[4].asString());

            HPEN pen = CreatePen(PS_SOLID, 2, col);
            HPEN oldPen = (HPEN)SelectObject(gfxHdcMem, pen);

            MoveToEx(gfxHdcMem, x1, y1, NULL);
            LineTo(gfxHdcMem, x2, y2);

            SelectObject(gfxHdcMem, oldPen);
            DeleteObject(pen);
        }
#endif
        return Value();
    });

    // gfx_draw_circle(cx, cy, radius, color)
    vm.defineNative("gfx_draw_circle", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (gfxHdcMem && argCount >= 4) {
            int cx = static_cast<int>(args[0].asNumber());
            int cy = static_cast<int>(args[1].asNumber());
            int r = static_cast<int>(args[2].asNumber());
            COLORREF col = parseColor(args[3].asString());

            HPEN pen = CreatePen(PS_SOLID, 2, col);
            HPEN oldPen = (HPEN)SelectObject(gfxHdcMem, pen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(gfxHdcMem, GetStockObject(NULL_BRUSH));

            Ellipse(gfxHdcMem, cx - r, cy - r, cx + r, cy + r);

            SelectObject(gfxHdcMem, oldPen);
            SelectObject(gfxHdcMem, oldBrush);
            DeleteObject(pen);
        }
#endif
        return Value();
    });

    // gfx_fill_circle(cx, cy, radius, color)
    vm.defineNative("gfx_fill_circle", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (gfxHdcMem && argCount >= 4) {
            int cx = static_cast<int>(args[0].asNumber());
            int cy = static_cast<int>(args[1].asNumber());
            int r = static_cast<int>(args[2].asNumber());
            COLORREF col = parseColor(args[3].asString());

            HPEN pen = CreatePen(PS_SOLID, 1, col);
            HBRUSH brush = CreateSolidBrush(col);
            HPEN oldPen = (HPEN)SelectObject(gfxHdcMem, pen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(gfxHdcMem, brush);

            Ellipse(gfxHdcMem, cx - r, cy - r, cx + r, cy + r);

            SelectObject(gfxHdcMem, oldPen);
            SelectObject(gfxHdcMem, oldBrush);
            DeleteObject(pen);
            DeleteObject(brush);
        }
#endif
        return Value();
    });

    // gfx_draw_text(x, y, text, color)
    vm.defineNative("gfx_draw_text", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (gfxHdcMem && argCount >= 4) {
            int x = static_cast<int>(args[0].asNumber());
            int y = static_cast<int>(args[1].asNumber());
            std::string text = args[2].asString();
            COLORREF col = parseColor(args[3].asString());

            SetBkMode(gfxHdcMem, TRANSPARENT);
            SetTextColor(gfxHdcMem, col);
            TextOutA(gfxHdcMem, x, y, text.c_str(), static_cast<int>(text.length()));
        }
#endif
        return Value();
    });

    // gfx_present()
    vm.defineNative("gfx_present", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        if (gfxHdcWindow && gfxHdcMem) {
            BitBlt(gfxHdcWindow, 0, 0, winWidth, winHeight, gfxHdcMem, 0, 0, SRCCOPY);
        }
#endif
        return Value();
    });

    // gfx_is_open()
    vm.defineNative("gfx_is_open", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        return Value(windowIsOpen);
#else
        return Value(false);
#endif
    });

    // gfx_poll_events()
    vm.defineNative("gfx_poll_events", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        MSG msg;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                windowIsOpen = false;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
#endif
        return Value();
    });

    // gfx_close()
    vm.defineNative("gfx_close", [](int argCount, const Value* args) -> Value {
#ifdef _WIN32
        windowIsOpen = false;
        if (gfxHdcMem) {
            if (gfxHbmOld) SelectObject(gfxHdcMem, gfxHbmOld);
            if (gfxHbmMem) DeleteObject(gfxHbmMem);
            DeleteDC(gfxHdcMem);
            gfxHdcMem = NULL;
            gfxHbmMem = NULL;
            gfxHbmOld = NULL;
        }
        if (gfxHdcWindow && gfxHwnd) {
            ReleaseDC(gfxHwnd, gfxHdcWindow);
            gfxHdcWindow = NULL;
        }
        if (gfxHwnd) {
            DestroyWindow(gfxHwnd);
            gfxHwnd = NULL;
        }
#endif
        return Value();
    });

}

} // namespace srl
