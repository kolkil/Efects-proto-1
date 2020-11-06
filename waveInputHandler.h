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
    const int sample_size = 16;
    const int buffers_per_sec = 1;
    std::unique_ptr<T[]> buffer;
    //std::unique_ptr<char[]> buffer;
    std::unique_ptr<T[]> current_sample_buffer;
    //std::unique_ptr<char[]> current_sample_buffer;
    WAVEHDR* headers = NULL;
    //std::unique_ptr<WAVEHDR[]> headers;
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
        waveInputHandler* current_handler = (waveInputHandler*)dwInstance;
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
    void cleanHeaders();
    std::condition_variable& getConditional();
    int getBurrefsNumber();

};

