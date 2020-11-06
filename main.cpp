#pragma comment(lib,"winmm.lib")
#include <Windows.h>
#include <string>
#include <mmsystem.h>

#include "waveInputHandler.h"
#include "amplifierEffect.h"
#include "delayEffect.h"
#include "bufferProcThread.h"
#include "waveOutputHandler.h"
#include "bufferDisplay.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
    MSG msg;

    waveInputHandler wih(10); // this should be much simpler and should implement interface

    amplifierEffect amef(1);
    delayEffect def(wih.getBurrefsNumber() / 2, wih.getCurrentBufferSize());
    waveOutputHandler woh(wih.makeOutFormatex(), wih.getBurrefsNumber());
    bufferDisplay bufDisplay(wih.getCurrentBufferSize(), "Raw buffer display");

    bufferProcThread bpt(&wih); // this is actualy main buffer processing thread and should take interface as argument

    // add effects
    //bpt.addEffect(&amef);
    //bpt.addEffect(&def);
    bpt.addEffect(&bufDisplay);

    // add last effect that will handle output
    bpt.addOutwriter(&woh);

    // open input device and add headers
    wih.openAndAddHeaders();

    // open output device
    woh.openWaveOut();

    bufDisplay.showWindowAndStartDrawing(hInstance, iCmdShow);

    // start output effect worker
    std::thread thrd(&bufferProcThread::work, &bpt);

    // start reading from input device
    wih.startWaveIn();

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // I should add some cleaning and releasing
    // this program is one big memory leak

    return 0;
}  // WinMain

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
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
}
