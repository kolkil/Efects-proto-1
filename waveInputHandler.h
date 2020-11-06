#pragma once
#pragma comment(lib,"winmm.lib")
#include <Windows.h>
#include <mmsystem.h>

#include <cstdlib>
#include <iostream>
#include <atomic>
#include <string>
#include <thread>
#include <array>
#include <condition_variable>

template <typename T>
class waveInputHandler
{
private:
    const int sps = 44100;
    const int sample_size = sizeof(T) * 8; // this can be char or short (1 or 2 bytes)
    const int buffers_per_sec = 1;
    std::unique_ptr<T[]> buffer;
    std::unique_ptr<T[]> current_sample_buffer;
    std::unique_ptr<WAVEHDR[]> headers;
    std::condition_variable cnd;
    size_t nbuffers;
    size_t single_buffer_len;
    size_t total_buffer_len;
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
        waveInputHandler<T>* current_handler = (waveInputHandler<T>*)dwInstance;
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
                current_handler->cnd.notify_one();
            }
            break;
        default:
            MessageBox(NULL, TEXT("Default case"), TEXT("TEST"), 0);
            break;
        }
    }

public:
    waveInputHandler(int nbuffps);
    T* getCurrentBuffer();
    void startWaveIn();
    void openAndAddHeaders();
    size_t getCurrentBufferSize();
    std::condition_variable& getCnd();
    WAVEFORMATEX makeOutFormatex();
    std::condition_variable& getConditional();
    int getBurrefsNumber();

};

template <typename T>
waveInputHandler<T>::waveInputHandler(int nbuffps)
{
    this->total_buffer_len = this->sps * this->sample_size / 8 / sizeof(T);
    this->nbuffers = nbuffps;
    this->single_buffer_len = total_buffer_len / nbuffers;
    buffer = std::make_unique<T[]>(total_buffer_len);
    current_sample_buffer = std::make_unique<T[]>(single_buffer_len);
    headers = std::make_unique<WAVEHDR[]>(nbuffers);
    memset(headers.get(), 0, nbuffps * sizeof(WAVEHDR));

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = sample_size;
    wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
}

template <typename T>
T* waveInputHandler<T>::getCurrentBuffer()
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

template <typename T>
void waveInputHandler<T>::startWaveIn()
{
    waveInStart(wi);
}

template <typename T>
void waveInputHandler<T>::openAndAddHeaders()
{
    waveInOpen(&wi,
        WAVE_MAPPER,
        &wfx,
        (DWORD_PTR)waveInputHandler::waveInProc, (DWORD_PTR)this,
        CALLBACK_FUNCTION
    );

    WAVEHDR* one_hdr;
    for (int i = 0; i < nbuffers; ++i)
    {
        one_hdr = &headers[i];
        //headers[i].lpData = buffer.get() + ((sps / nbuffers) * i * (sample_size / 8));
        //headers[i].lpData = (char*)(buffer.get() + ((sps / nbuffers) * i * (sample_size / 8)));
        one_hdr->lpData = (char*)&buffer[single_buffer_len * i];
        one_hdr->dwBufferLength = single_buffer_len * sizeof(T);
        one_hdr->dwUser = 1;
        waveInPrepareHeader(wi, one_hdr, sizeof(WAVEHDR));
        waveInAddBuffer(wi, one_hdr, sizeof(WAVEHDR));
    }
}

template <typename T>
size_t waveInputHandler<T>::getCurrentBufferSize()
{
    return single_buffer_len;
}

template <typename T>
std::condition_variable& waveInputHandler<T>::getCnd()
{
    return cnd;
}

template <typename T>
WAVEFORMATEX waveInputHandler<T>::makeOutFormatex()
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

template <typename T>
std::condition_variable& waveInputHandler<T>::getConditional()
{
    return cnd;
}

template <typename T>
int waveInputHandler<T>::getBurrefsNumber()
{
    return nbuffers;
}

