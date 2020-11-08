#pragma comment(lib,"winmm.lib")
#include <Windows.h>
#include <string>
#include <mmsystem.h>

#include "waveInBufferProvider.h"
#include "amplifierEffect.h"
#include "delayEffect.h"
#include "pipelineProcessor.h"
#include "waveOutputHandler.h"
#include "bufferDisplay.h"
#include "fftEffect.h"
#include "distortionEffect.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
    MSG msg;

    // buffer provider, gets buffers from default audio input device
    waveInBufferProvider<short> wibp(4096, 10);

    // simple effects
    amplifierEffect<short> amef(1);
    delayEffect<short> def(wibp.getNBuffers(), wibp.getBufferLen());
    distortionEffect<short> distorf(0.01);

    // display effects
    bufferDisplay<short> bufDisplay(wibp.getBufferLen(), "Raw buffer display");
    bufferDisplay<short> fftDisplay(wibp.getBufferLen(), "FFT display");

    // analytic effects
    fftEffect<short> fftf(wibp.getBufferLen());

    // output writer effect, writes data to default audio output device
    waveOutputHandler<short> woh(wibp.makeOutFormatex(), wibp.getNBuffers());

    // pipeline processor, waits for buffer from pbuffer provider and applies/executes all the effects
    pipelineProcessor<short> pps(&wibp);

    // add effect to fft display to actualy perform fft before displaying
    fftDisplay.addEffect(&fftf);

    // add effects to main thread
    //bpt.addEffect(&amef);
    //bpt.addEffect(&def);
    //pps.addEffect(&distorf); // before displaying add distortion
    pps.addEffect(&bufDisplay); // displays are not modifying values in buffer in main pipline
    pps.addEffect(&fftDisplay);

    // add last effect that will handle output
    pps.addLastEffect(&woh);

    // open input device and add headers
    wibp.openAndAddHeaders();

    // open output device
    woh.openWaveOut();

    bufDisplay.showWindowAndStartDrawing(hInstance, iCmdShow);
    fftDisplay.showWindowAndStartDrawing(hInstance, iCmdShow);

    // start pipeline
    pps.start();

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

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
