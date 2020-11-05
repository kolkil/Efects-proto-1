#pragma once

#include <Windows.h>
#include <mmsystem.h>
#include <iostream>

#include "IEffect.h"

class waveOutputHandler :
    public IEffect
{
    HWAVEOUT hWaveOut = { 0 };
    WAVEFORMATEX wfxout = { 0 };
    WAVEHDR* headers = NULL;
    size_t nheaders = 0;
    int header_id = -1;

    static void CALLBACK waveOutProc(
        HWAVEOUT  hwo,
        UINT      uMsg,
        DWORD_PTR dwInstance,
        DWORD_PTR dwParam1,
        DWORD_PTR dwParam2
    )
    {
        WAVEHDR* hdr = (WAVEHDR*)dwParam1;
        switch (uMsg)
        {
        case WOM_DONE:
            hdr->dwUser = 2;
            break;
        default:
            break;
        }
    }

public:
    waveOutputHandler(WAVEFORMATEX wfx, size_t nheaders);
    void openWaveOut();
    void writeOut(char* buffer, size_t size);
    void apply(char* buffer, size_t size);
};
