#include "waveOutputHandler.h"

waveOutputHandler::waveOutputHandler(WAVEFORMATEX wfx, size_t nheaders)
{
    wfxout = wfx;
    headers = new WAVEHDR[nheaders];
    memset(headers, 0, sizeof(WAVEHDR) * nheaders);
    this->nheaders = nheaders;
    header_id = 0;
}

void waveOutputHandler::openWaveOut()
{
    int r = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfxout, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
}

void waveOutputHandler::writeOut(char* buffer, size_t size)
{
    //char out_buffer[8820] = { 0 };
    //memset(out_buffer, 100, 8820);
    WAVEHDR* out_hdr = headers;
    for (int i = header_id, c = 0; c < nheaders; i = i >= nheaders- 1 ? 0 : i + 1, c++)
    {
        if ((int)headers[i].dwUser == 2)
        {
            waveOutUnprepareHeader(hWaveOut, headers + i, sizeof(WAVEHDR));
            headers[i].dwUser = 0;
        }
        if ((int)headers[i].dwUser == 0)
        {
            out_hdr = headers + i;
            out_hdr->dwUser = 1;
            break;
        }
    }
    out_hdr->lpData = buffer;
    out_hdr->dwBufferLength = (int)size;
    waveOutPrepareHeader(hWaveOut, out_hdr, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, out_hdr, sizeof(WAVEHDR));
}

void waveOutputHandler::apply(char* buffer, size_t size)
{
    writeOut(buffer, size);
}
