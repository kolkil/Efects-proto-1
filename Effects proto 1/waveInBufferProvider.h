#pragma once

#pragma comment(lib,"winmm.lib")
#include <Windows.h>
#include <mmsystem.h>

#include <cstdlib>
#include <iostream>
#include <condition_variable>
#include <mutex>

#include "IBufferProvider.h"

template <typename T>
class waveInBufferProvider :
    public IBufferProvider<T>
{
    const int samples_per_second = 44100;
    const int sample_size = sizeof(T) * 8; // this can be char or short (1 or 2 bytes)
    std::unique_ptr<T[]> buffer;
    std::unique_ptr<T[]> current_sample_buffer;
    std::unique_ptr<WAVEHDR[]> headers;
    std::condition_variable is_any_buffer_ready_cnd;
    size_t nbuffers;
    size_t single_buffer_len;
    size_t total_buffer_len_samples;
    size_t total_buffer_len_bytes;
    WAVEFORMATEX wfx = {};
    HWAVEIN wi = {};
    int current_header_id = 0;

    static void CALLBACK waveInProc(
        HWAVEIN   hwi,
        UINT      uMsg,
        DWORD_PTR dwInstance,
        DWORD_PTR dwParam1,
        DWORD_PTR dwParam2
    )
    {
        waveInBufferProvider<T>* current_handler = (waveInBufferProvider<T>*)dwInstance;
        WAVEHDR* hdr = (WAVEHDR*)dwParam1;
        switch (uMsg)
        {
        case WIM_CLOSE:
            //MessageBox(NULL, TEXT("WIM_CLOSE"), TEXT("WIM_CLOSE"), 0);
            break;
        case WIM_OPEN:
            //MessageBox(NULL, TEXT("WIM_OPEN"), TEXT("WIM_OPEN"), 0);
            break;
        case WIM_DATA:
            //MessageBox(NULL, (LPCWSTR)std::to_wstring((int)current_handler->sps).c_str(), TEXT("WIM_DATA"), 0);
            if (hdr->dwFlags & WHDR_DONE && hdr && dwInstance)
            {
                hdr->dwUser = 2;
                current_handler->is_any_buffer_ready_cnd.notify_one();
            }
            break;
        default:
            //MessageBox(NULL, TEXT("Default case"), TEXT("TEST"), 0);
            break;
        }
    }

public:
    waveInBufferProvider(size_t buffer_size, int nbuffs);
    T* getBuffer();
    void startWaveIn();
    void openAndAddHeaders();
    void waitForBuffer();
    //size_t getSingleBufferSize();
    size_t getBufferLen();
    size_t getBufferSize();
    //std::condition_variable& getCnd();
    WAVEFORMATEX makeOutFormatex();
    //std::condition_variable& getConditional();
    int getNBuffers();
    void start();

};

template<typename T>
inline waveInBufferProvider<T>::waveInBufferProvider(size_t buffer_size, int nbuffs)
{
    this->single_buffer_len = buffer_size;
    this->nbuffers = nbuffs;
    this->total_buffer_len_samples = nbuffs * buffer_size;
    this->total_buffer_len_bytes = total_buffer_len_samples * sizeof(T);

    buffer = std::make_unique<T[]>(total_buffer_len_samples);
    current_sample_buffer = std::make_unique<T[]>(single_buffer_len);
    headers = std::make_unique<WAVEHDR[]>(nbuffers);
    memset(headers.get(), 0, nbuffs * sizeof(WAVEHDR));

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = sample_size;
    wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
}

template<typename T>
inline T* waveInBufferProvider<T>::getBuffer()
{
    for (int i = current_header_id, c = 0; c < nbuffers; i = i >= nbuffers - 1 ? 0 : i + 1, c++)
    {
        WAVEHDR* current_hdr = &headers[i];
        if (current_hdr->dwUser == 2)
        {
            memcpy(current_sample_buffer.get(), current_hdr->lpData, current_hdr->dwBufferLength);
            current_hdr->dwFlags = 0;
            current_hdr->dwBytesRecorded = 0;
            current_hdr->dwUser = 1;
            waveInPrepareHeader(wi, current_hdr, sizeof(WAVEHDR));
            waveInAddBuffer(wi, current_hdr, sizeof(WAVEHDR));
            current_header_id = i;
            break;
        }
    }
    return current_sample_buffer.get();
}

template<typename T>
inline void waveInBufferProvider<T>::startWaveIn()
{
    waveInStart(wi);
}

template<typename T>
inline void waveInBufferProvider<T>::openAndAddHeaders()
{
    waveInOpen(&wi,
        WAVE_MAPPER,
        &wfx,
        (DWORD_PTR)waveInBufferProvider::waveInProc, (DWORD_PTR)this,
        CALLBACK_FUNCTION
    );

    WAVEHDR* one_hdr;
    for (int i = 0; i < nbuffers; ++i)
    {
        one_hdr = &headers[i];
        one_hdr->lpData = (char*)&buffer[single_buffer_len * i];
        one_hdr->dwBufferLength = single_buffer_len * sizeof(T);
        one_hdr->dwUser = 1;
        waveInPrepareHeader(wi, one_hdr, sizeof(WAVEHDR));
        waveInAddBuffer(wi, one_hdr, sizeof(WAVEHDR));
    }
}

template<typename T>
inline void waveInBufferProvider<T>::waitForBuffer()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> ulck(mtx);
    is_any_buffer_ready_cnd.wait(ulck);
    ulck.unlock();
}

template<typename T>
inline size_t waveInBufferProvider<T>::getBufferLen()
{
    return single_buffer_len;
}

template<typename T>
inline size_t waveInBufferProvider<T>::getBufferSize()
{
    return getBufferLen() * sizeof(T);
}

template<typename T>
inline WAVEFORMATEX waveInBufferProvider<T>::makeOutFormatex()
{
    WAVEFORMATEX wfxout = { 0 };
    wfxout.nChannels = 1;
    wfxout.wBitsPerSample = sample_size;
    wfxout.nSamplesPerSec = 44100;
    wfxout.wFormatTag = WAVE_FORMAT_PCM;
    wfxout.nBlockAlign = wfxout.wBitsPerSample * wfxout.nChannels / 8;
    wfxout.nAvgBytesPerSec = wfxout.nBlockAlign * wfxout.nSamplesPerSec;

    return wfxout;
}

template<typename T>
inline int waveInBufferProvider<T>::getNBuffers()
{
    return nbuffers;
}

template<typename T>
inline void waveInBufferProvider<T>::start()
{
    startWaveIn();
}
