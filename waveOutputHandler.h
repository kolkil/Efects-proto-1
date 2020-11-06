#pragma once

#include <Windows.h>
#include <mmsystem.h>
#include <iostream>

#include "IEffect.h"

template<typename T>
class waveOutputHandler :
    public IEffect<T>
{
    HWAVEOUT hWaveOut = { 0 };
    WAVEFORMATEX wfxout = { 0 };
    //WAVEHDR* headers = NULL;
    std::unique_ptr<WAVEHDR[]> headers;
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
    void writeOut(T* buffer, size_t size);
    void apply(T* buffer, size_t size);
};

template<typename T>
waveOutputHandler<T>::waveOutputHandler(WAVEFORMATEX wfx, size_t nheaders)
{
    wfxout = wfx;
    headers = std::make_unique<WAVEHDR[]>(nheaders);
    memset(headers.get(), 0, sizeof(WAVEHDR) * nheaders);
    this->nheaders = nheaders;
    header_id = 0;
}

template<typename T>
void waveOutputHandler<T>::openWaveOut()
{
    int r = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfxout, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
}

template<typename T>
void waveOutputHandler<T>::writeOut(T* buffer, size_t size)
{
    WAVEHDR* out_hdr = headers.get();
    for (int i = header_id, c = 0; c < nheaders; i = i >= nheaders - 1 ? 0 : i + 1, c++)
    {
        if ((int)headers[i].dwUser == 2)
        {
            waveOutUnprepareHeader(hWaveOut, &headers[i], sizeof(WAVEHDR));
            headers[i].dwUser = 0;
        }
        if ((int)headers[i].dwUser == 0)
        {
            out_hdr = &headers[i];
            out_hdr->dwUser = 1;
            break;
        }
    }
    out_hdr->lpData = (char*)buffer;
    out_hdr->dwBufferLength = (int)size * sizeof(T);
    waveOutPrepareHeader(hWaveOut, out_hdr, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, out_hdr, sizeof(WAVEHDR));
}

template<typename T>
void waveOutputHandler<T>::apply(T* buffer, size_t size)
{
    writeOut(buffer, size);
}
