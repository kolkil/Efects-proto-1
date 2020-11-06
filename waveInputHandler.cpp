#include "waveInputHandler.h"


waveInputHandler::waveInputHandler(int nbuffps)
{
    this->total_buffer_len = this->sps * this->sample_size / 8;
    this->nbuffers = nbuffps;
    this->single_buffer_len = total_buffer_len / nbuffers;
    //buffer = std::make_unique<char[]>(total_buffer_len);
    buffer = new char[total_buffer_len];
    //current_sample_buffer = std::make_unique<char[]>(single_buffer_len);
    current_sample_buffer = new char[single_buffer_len];
    headers = new WAVEHDR[nbuffers];
    //headers = std::make_unique<WAVEHDR[]>(nbuffps);
    memset(headers, 0, nbuffps * sizeof(WAVEHDR));

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = sample_size;
    wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
}

//void waveInputHandler::setCurrentBuffer(char* buff)
//{
//    memcpy(current_sample_buffer, buff, single_buffer_len);
//    //memcpy(current_sample_buffer.get(), buff, single_buffer_len);
//}

char* waveInputHandler::getCurrentBuffer()
{
    for (int i = current_header_id, c = 0; c < nbuffers; i = i >= nbuffers - 1 ? 0 : i + 1, c++)
    {
        if (headers[i].dwUser == 2)
        {
            memcpy(current_sample_buffer, headers[i].lpData, headers[i].dwBufferLength);
            headers[i].dwFlags = 0;
            headers[i].dwBytesRecorded = 0;
            headers[i].dwUser = 1;
            waveInPrepareHeader(wi, &headers[i], sizeof(WAVEHDR));
            waveInAddBuffer(wi, &headers[i], sizeof(WAVEHDR));
            current_header_id = i;
            break;
        }
    }
    return current_sample_buffer;
    //return current_sample_buffer.get();
}

void waveInputHandler::startWaveIn()
{
    waveInStart(wi);
}

void waveInputHandler::openAndAddHeaders()
{
    waveInOpen(&wi,
        WAVE_MAPPER,
        &wfx,
        (DWORD_PTR)waveInputHandler::waveInProc, (DWORD_PTR)this,
        CALLBACK_FUNCTION
    );

    for (int i = 0; i < nbuffers; ++i)
    {
        //headers[i].lpData = buffer.get() + ((sps / nbuffers) * i * (sample_size / 8));
        headers[i].lpData = buffer + ((sps / nbuffers) * i * (sample_size / 8));
        headers[i].dwBufferLength = (sps / nbuffers) * (sample_size / 8);
        headers[i].dwUser = 1;
        waveInPrepareHeader(wi, &headers[i], sizeof(headers[i]));
        waveInAddBuffer(wi, &headers[i], sizeof(headers[i]));
    }
}

size_t waveInputHandler::getCurrentBufferSize()
{
    return single_buffer_len;
}

std::condition_variable& waveInputHandler::getCnd()
{
    return cnd;
}

WAVEFORMATEX waveInputHandler::makeOutFormatex()
{
    WAVEFORMATEX wfxout = { 0 };
    wfxout.nChannels = 1;
    wfxout.wBitsPerSample = 16;
    wfxout.nSamplesPerSec = 44100;
    wfxout.wFormatTag = WAVE_FORMAT_PCM;
    wfxout.nBlockAlign = wfxout.wBitsPerSample * wfxout.nChannels / 8;
    wfxout.nAvgBytesPerSec = wfxout.nBlockAlign * wfxout.nSamplesPerSec;

    return wfxout;
}

void waveInputHandler::cleanHeaders()
{
    for (int i = 0; i < nbuffers; ++i)
    {
        if ((int)headers[i].dwUser == 2)
        {
            headers[i].dwUser = 1;
            waveInPrepareHeader(wi, headers + i, sizeof(headers[i]));
            waveInAddBuffer(wi, headers + i, sizeof(headers[i]));
        }
    }
}

std::condition_variable& waveInputHandler::getConditional()
{
    return cnd;
}

int waveInputHandler::getBurrefsNumber()
{
    return nbuffers;
}
