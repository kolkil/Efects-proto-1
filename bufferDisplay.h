#pragma once
#pragma comment (lib,"Gdiplus.lib")
#include <Windows.h>
#include <gdiplus.h>

#include "IEffect.h"

#include <thread>
#include <condition_variable>
#include <vector>
#include <string>

class bufferDisplay :
    public IEffect
{
private:
    std::unique_ptr<std::thread> drawer;
    size_t buffer_length;
    std::unique_ptr<char[]> buffer;
    std::condition_variable cnd;
    std::vector<IEffect*> effects;
    std::unique_ptr<Gdiplus::Graphics> graphics;
    std::unique_ptr<Gdiplus::Pen> pen;
    HDC hdc;
    HWND hWnd;
    WNDCLASS wndClass;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    HINSTANCE hInstance;
    std::string windowName;

    void drawBuffer();
    static LRESULT CALLBACK bufferDisplayWndProc(HWND hWnd, UINT message,
        WPARAM wParam, LPARAM lParam)
    {

        switch (message)
        {
        case WM_PAINT:
            return 0;
        case WM_DESTROY:
            return 0;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    } // WndProc

public:
    bufferDisplay(size_t buffer_size, std::string classname);
    void addEffect(IEffect* e);
    void apply(char* buffer, size_t buffer_len);
    void showWindowAndStartDrawing(HINSTANCE hInstance, INT iCmdShow);
};

