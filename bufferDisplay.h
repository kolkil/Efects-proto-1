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
    void addEffect(IEffect<T>* e);
    void apply(T* buffer, size_t buffer_len);
    void showWindowAndStartDrawing(HINSTANCE hInstance, INT iCmdShow);
};


template <typename T>
void bufferDisplay<T>::drawBuffer()
{
    while (1)
    {
        std::mutex mtx;
        std::unique_lock<std::mutex> ulck(mtx);
        cnd.wait(ulck);
        int height = 0;
        int width = 0;
        int maxT = (std::numeric_limits<T>::max)();
        int minT = (std::numeric_limits<T>::min)();
        RECT r = { 0 };
        GetWindowRect(hWnd, &r);
        height = r.bottom - r.top;
        width = r.right - r.left;
        double scale_factor = ((double)height) / ((double)maxT);
        double step = ((double)buffer_length / (double)width);
        graphics->Clear(Gdiplus::Color(255, 255, 255));

        double maxi = (std::numeric_limits<short>::min)();

        for (int i = 0; i < buffer_length; ++i)
        {
            maxi = (std::max)(maxi, (double)buffer[i]);
        }

        for (int i = 0, idx = 0; i < width; ++i, idx += step)
        {
            graphics->DrawLine(pen.get(), i, (double(buffer[idx]) / maxi) * (height / 2) + height / 2, i + 1, (double(buffer[idx + step]) / maxi) * (height / 2) + height / 2);
        }

        ulck.unlock();
    }
}

template <typename T>
bufferDisplay<T>::bufferDisplay(size_t buffer_size, std::string classname)
{
    drawer = std::make_unique<std::thread>(&bufferDisplay::drawBuffer, this);
    buffer = std::make_unique<T[]>(buffer_size);
    windowName = classname;
    buffer_length = buffer_size;
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
        GetSystemMetrics(SM_CXSCREEN),            // initial x size
        GetSystemMetrics(SM_CYSCREEN),            // initial y size
        NULL,                     // parent window handle
        NULL,                     // window menu handle
        hInstance,                // program instance handle
        NULL);                    // creation parameters

    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);
    hdc = BeginPaint(hWnd, &ps);
    graphics = std::make_unique<Gdiplus::Graphics>(hdc);
    pen = std::make_unique<Gdiplus::Pen>(Gdiplus::Color(255, 0, 0, 255));
}

