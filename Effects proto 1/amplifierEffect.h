#pragma once
#include "IEffect.h"

template <typename T>
class amplifierEffect : public IEffect<T>
{
private:
    float multiply = 1.0;
public:

    amplifierEffect(float scale);
    virtual void apply(T* buffer, size_t size);
};

template <typename T>
amplifierEffect<T>::amplifierEffect(float scale)
{
    multiply = scale;
}

template <typename T>
void amplifierEffect<T>::apply(T* buffer, size_t size)
{
    for (int i = 0; i < size; ++i)
    {
        buffer[i] *= multiply;
    }
}
