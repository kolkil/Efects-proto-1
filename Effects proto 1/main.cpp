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
#include "fftEffect.h"
#include "distortionEffect.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);


// sygna³ oknem Hamminga o d³ugoœci bufora
// fft czêœæ urojona zerowa
// 20 * log (|wynik|)

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
    MSG msg;

    // this should be much simpler and should implement interface
    waveInputHandler<short> wih(4096, 10);

    // simple effects
    amplifierEffect<short> amef(1);
    delayEffect<short> def(wih.getBurrefsNumber() / 2, wih.getSingleBufferSize());
    distortionEffect<short> distorf(0.01);

    // display effects
    bufferDisplay<short> bufDisplay(wih.getSingleBufferSize(), "Raw buffer display");
    bufferDisplay<short> fftDisplay(wih.getSingleBufferSize(), "FFT display");

    // analytic effects
    fftEffect<short> fftf(wih.getSingleBufferSize());

    // output writer effect
    waveOutputHandler<short> woh(wih.makeOutFormatex(), wih.getBurrefsNumber());

    // main processing thread
    bufferProcThread<short> bpt(&wih); // this is actualy main buffer processing thread and should take interface as argument

    // add effect to fft display to actualy perform fft before displaying
    fftDisplay.addEffect(&fftf);

    // add effects to main thread
    //bpt.addEffect(&amef);
    //bpt.addEffect(&def);
    bpt.addEffect(&distorf); // before displaying add distortion
    bpt.addEffect(&bufDisplay); // displeys are not modifying values in buffer in main pipline
    bpt.addEffect(&fftDisplay);

    // add last effect that will handle output
    bpt.addOutwriter(&woh);

    // open input device and add headers
    wih.openAndAddHeaders();

    // open output device
    woh.openWaveOut();

    bufDisplay.showWindowAndStartDrawing(hInstance, iCmdShow);
    fftDisplay.showWindowAndStartDrawing(hInstance, iCmdShow);

    // start output effect worker
    std::thread thrd(&bufferProcThread<short>::work, &bpt);

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
