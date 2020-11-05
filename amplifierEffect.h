#pragma once
#include "IEffect.h"

class amplifierEffect : public IEffect
{
private:
    float multiply = 1.0;
public:

    amplifierEffect(float scale);
    virtual void apply(char* buffer, size_t size);
};

