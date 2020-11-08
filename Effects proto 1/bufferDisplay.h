#pragma once
#pragma comment (lib,"Gdiplus.lib")
#include <Windows.h>
#include <gdiplus.h>

#include "IEffect.h"

#include <thread>
#include <condition_variable>
#include <vector>
#include <string>
#include <mutex>
#include <limits>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdlib>

template<typename T>
class bufferDisplay :
    public IEffect<T>
{
private:
    std::unique_ptr<std::thread> drawer;
    size_t buffer_length;
    std::unique_ptr<T[]> buffer;
    std::condition_variable cnd;
    std::vector<IEffect<T>*> effects;
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
    void addEffect(IEffect<T>* e);
    void apply(T* buffer, size_t buffer_len);
    void showWindowAndStartDrawing(HINSTANCE hInstance, INT iCmdShow);
};


template <typename T>
void bufferDisplay<T>::drawBuffer()
{
    std::unique_ptr<T[]> local_buffer_copy = std::make_unique<T[]>(buffer_length);
    std::unique_ptr<Gdiplus::Graphics> graphics;
    int height = 0;
    int width = 0;

    while (1)
    {
        std::mutex mtx;
        std::unique_lock<std::mutex> ulck(mtx);

        RECT r = { 0 };

        int new_height = 0;
        int new_width = 0;
        int axis0_v = 0; // 0 axis vertical move
        int idx1 = 0;
        int idx2 = 0;
        double scale_factor = 0;
        double maxi = (std::numeric_limits<T>::min)();
        double mini = (std::numeric_limits<T>::max)();
        double absMaxi = 0;
        double step = 0;

        cnd.wait(ulck);

        if (!graphics.get())
        {
            graphics = std::make_unique<Gdiplus::Graphics>(hdc);
        }

        memcpy(local_buffer_copy.get(), buffer.get(), buffer_length * sizeof(T));

        GetWindowRect(hWnd, &r);

        new_height = r.bottom - r.top - 50; // - some pixels for general better look
        new_width = r.right - r.left;

        if (new_height != height || new_width != width) // if window was resized, recreate graphics
        {
            graphics.release();
            graphics = std::make_unique<Gdiplus::Graphics>(hdc);
            height = new_height;
            width = new_width;
        }

        axis0_v = height / 2;
        step = ((double)buffer_length / (double)width);

        graphics->Clear(Gdiplus::Color(255, 255, 255));

        for (int i = 0; i < buffer_length; ++i)
        {
            maxi = (std::max)(maxi, (double)local_buffer_copy[i]);
            mini = (std::min)(mini, (double)local_buffer_copy[i]);
        }

        absMaxi = (std::max)(std::abs(maxi), std::abs(mini));
        scale_factor = ((double)axis0_v) / ((double)absMaxi);

        for (int i = 0; i < width - 1; ++i)
        {
            idx1 = std::round(double(i) * step);
            idx2 = std::round(double(i + 1) * step);
            graphics->DrawLine(pen.get(), i, double(local_buffer_copy[idx1]) * scale_factor + axis0_v, i + 1, double(local_buffer_copy[idx2]) * scale_factor + axis0_v);
        }

        ulck.unlock();
    }
}

template <typename T>
bufferDisplay<T>::bufferDisplay(size_t buffer_size, std::string classname)
{
    buffer = std::make_unique<T[]>(buffer_size);
    windowName = classname;
    buffer_length = buffer_size;
    drawer = std::make_unique<std::thread>(&bufferDisplay::drawBuffer, this);
}

template <typename T>
void bufferDisplay<T>::addEffect(IEffect<T>* e)
{
    effects.push_back(e);
}

template <typename T>
void bufferDisplay<T>::apply(T* buffer, size_t len)
{
    memcpy(this->buffer.get(), buffer, this->buffer_length * sizeof(T));
    for (IEffect<T>* e : effects)
    {
        e->apply(this->buffer.get(), len);
    }
    cnd.notify_one();
}

template <typename T>
void bufferDisplay<T>::showWindowAndStartDrawing(HINSTANCE hInstance, INT iCmdShow)
{
    PAINTSTRUCT  ps = { 0 };
    std::wstring wWindowName(this->windowName.begin(), this->windowName.end());
    this->hInstance = hInstance;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = bufferDisplayWndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = TEXT("bufferDisplay");

    RegisterClass(&wndClass);
    hWnd = CreateWindow(
        TEXT("bufferDisplay"),
        wWindowName.c_str(),   // window class name
        WS_OVERLAPPEDWINDOW,      // window style
        CW_USEDEFAULT,            // initial x position
        CW_USEDEFAULT,            // initial y position
        //GetSystemMetrics(SM_CXSCREEN),            // initial x size
        //GetSystemMetrics(SM_CXSCREEN),            // initial x size
        CW_USEDEFAULT,            // initial y size
        CW_USEDEFAULT,            // initial y size
        NULL,                     // parent window handle
        NULL,                     // window menu handle
        hInstance,                // program instance handle
        NULL);                    // creation parameters

    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);
    hdc = BeginPaint(hWnd, &ps);
    pen = std::make_unique<Gdiplus::Pen>(Gdiplus::Color(255, 0, 0, 255));
}
