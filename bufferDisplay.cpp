#include <mutex>

#include "bufferDisplay.h"

void bufferDisplay::drawBuffer()
{
    while (1)
    {
        std::mutex mtx;
        std::unique_lock<std::mutex> ulck(mtx);
        cnd.wait(ulck);
        int height = 0;
        int width = 0;
        RECT r = { 0 };
        GetWindowRect(hWnd, &r);
        height = r.bottom - r.top;
        width = r.right - r.left;
        graphics->Clear(Gdiplus::Color(255, 255, 255));

        for (int i = 0; i < width; ++i)
        {
            graphics->DrawLine(pen.get(), i, buffer[i * (buffer_length / width)] + height / 2, i + 1, buffer[(i + 1) * (buffer_length / width)] + height / 2);
        }

        ulck.unlock();
    }
}

bufferDisplay::bufferDisplay(size_t buffer_size, std::string classname)
{
    drawer = std::make_unique<std::thread>(&bufferDisplay::drawBuffer, this);
    buffer = std::make_unique<char[]>(buffer_size);
    windowName = classname;
    buffer_length = buffer_size;
}

void bufferDisplay::addEffect(IEffect* e)
{
    effects.push_back(e);
}

void bufferDisplay::apply(char* buffer, size_t len)
{
    memcpy(this->buffer.get(), buffer, this->buffer_length);
    for (IEffect* e : effects)
    {
        e->apply(this->buffer.get(), len);
    }
    cnd.notify_one();
}

void bufferDisplay::showWindowAndStartDrawing(HINSTANCE hInstance, INT iCmdShow)
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
