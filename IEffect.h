#pragma once

class IEffect
{
public:
    virtual void apply(char* buffer, size_t size) = 0;
};
