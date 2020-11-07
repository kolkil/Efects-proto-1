#pragma once
#include <condition_variable>
#include <vector>
#include <mutex>
#include <thread>
#include "IEffect.h"
#include "waveInputHandler.h"

template <typename T>
class bufferProcThread
{
private:
    std::condition_variable& cnd;
    std::unique_ptr<T[]> buffer;
    size_t buffer_len = 0;
    std::vector<IEffect<T>*> effects;
    std::unique_ptr<std::thread> thrd;
    IEffect<T>* outwriter = nullptr;
    waveInputHandler<T>* waveIn;

public:
    bufferProcThread(waveInputHandler<T>* wih) :
        cnd(wih->getConditional())
    {
        waveIn = wih;
        buffer_len = waveIn->getSingleBufferSize();
        buffer = std::make_unique<T[]>(buffer_len);
    }

    void addEffect(IEffect<T>* e)
    {
        effects.push_back(e);
    }

    void addOutwriter(IEffect<T>* out)
    {
        outwriter = out;
    }

    void work();
};

template <typename T>
void bufferProcThread<T>::work()
{
    while (1)
    {
        std::mutex mtx;
        std::unique_lock<std::mutex> ulck(mtx);
        cnd.wait(ulck);
        memcpy(buffer.get(), waveIn->getCurrentBuffer(), buffer_len * sizeof(T));

        for (int i = 0; i < effects.size(); ++i)
        {
            effects[i]->apply(buffer.get(), buffer_len);
        }

        if (outwriter)
        {
            outwriter->apply(buffer.get(), buffer_len);
        }

        ulck.unlock();
    }
}
