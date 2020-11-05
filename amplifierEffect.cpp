#include "amplifierEffect.h"

amplifierEffect::amplifierEffect(float scale)
{
    multiply = scale;
}

void amplifierEffect::apply(char* buffer, size_t size)
{
    for (int i = 0; i < size; ++i)
    {
        buffer[i] *= multiply;
    }
}
