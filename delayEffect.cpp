#include "delayEffect.h"

delayEffect::delayEffect(size_t nbuffers, size_t single_buffer_size)
{
    this->single_buffer_size = single_buffer_size;
    this->nbuffers = nbuffers;

    for (int i = 0; i < nbuffers; ++i)
    {
        buffers.push_back(std::make_unique<char[]>(single_buffer_size));
    }
}

void delayEffect::apply(char* buffer, size_t len)
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
