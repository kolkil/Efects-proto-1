#pragma once
#include <condition_variable>
#include <vector>
#include "IEffect.h"
#include "waveInputHandler.h"

class bufferProcThread
{
private:
    std::condition_variable& cnd;
    char* buffer = NULL;
    size_t buffer_len = 0;
    std::vector<IEffect*> effects;
    std::unique_ptr<std::thread> thrd;
    IEffect* outwriter = nullptr;
    waveInputHandler* waveIn;

public:
    bufferProcThread(waveInputHandler *wih) :
        cnd(wih->getConditional())
    {
        waveIn = wih;
        buffer_len = waveIn->getCurrentBufferSize();
        buffer = new char[buffer_len];
    }

    void addEffect(IEffect* e)
    {
        effects.push_back(e);
    }

    void addOutwriter(IEffect* out)
    {
        outwriter = out;
    }

    void start();
    void work();
};
