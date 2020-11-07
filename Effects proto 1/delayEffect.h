#pragma once
#include "IEffect.h"

#include <vector>
#include <iostream>

template <typename T>
class delayEffect :
    public IEffect<T>
{
private:
    std::vector<std::unique_ptr<T[]>> buffers;
    size_t single_buffer_size = 0;
    size_t nbuffers = 0;
    int out_buffer = 0;
    int in_buffer = 0;
    bool ready_flag = false;

public:
    delayEffect(size_t nbuffers, size_t single_buffer_size);
    void apply(T* buffer, size_t len);
};

template <typename T>
delayEffect<T>::delayEffect(size_t nbuffers, size_t single_buffer_size)
{
    this->single_buffer_size = single_buffer_size;
    this->nbuffers = nbuffers;

    for (int i = 0; i < nbuffers; ++i)
    {
        buffers.push_back(std::make_unique<T[]>(single_buffer_size));
    }
}

template <typename T>
void delayEffect<T>::apply(T* buffer, size_t len)
{
    memcpy(buffers[in_buffer++].get(), buffer, single_buffer_size);

    if (in_buffer >= nbuffers)
    {
        ready_flag = true;
        in_buffer = 0;
    }

    if (ready_flag)
    {
        for (int i = 0; i < single_buffer_size; ++i)
        {
            buffer[i] += (buffers[out_buffer].get())[i];
            buffer[i] /= 2;
        }
        out_buffer = out_buffer >= nbuffers - 1 ? 0 : out_buffer + 1;
    }
}
