#pragma once
#include <condition_variable>
#include <vector>
#include <mutex>
#include <thread>

#include "IEffect.h"
#include "IBufferProvider.h"

template <typename T>
class pipelineProcessor
{
private:
    std::unique_ptr<T[]> buffer;
    size_t buffer_len = 0;
    std::vector<IEffect<T>*> effects;
    std::unique_ptr<std::thread> thrd;
    IEffect<T>* last_effect = nullptr;
    IBufferProvider<T>* buffer_provider;

public:
    pipelineProcessor(IBufferProvider<T>* buffer_provider)
    {
        this->buffer_provider = buffer_provider;
        buffer_len = buffer_provider->getBufferLen();
        buffer = std::make_unique<T[]>(buffer_len);
    }

    void addEffect(IEffect<T>* e)
    {
        effects.push_back(e);
    }

    void addLastEffect(IEffect<T>* out)
    {
        last_effect = out;
    }

    void work();
    void start();
};

template <typename T>
void pipelineProcessor<T>::work()
{
    buffer_provider->start();
    while (1)
    {
        buffer_provider->waitForBuffer();
        memcpy(buffer.get(), buffer_provider->getBuffer(), buffer_provider->getBufferSize());

        for (int i = 0; i < effects.size(); ++i)
        {
            effects[i]->apply(buffer.get(), buffer_len);
        }

        if (last_effect)
        {
            last_effect->apply(buffer.get(), buffer_len);
        }
    }
}

template<typename T>
inline void pipelineProcessor<T>::start()
{
    thrd = std::make_unique<std::thread>(&pipelineProcessor::work, this);
}
