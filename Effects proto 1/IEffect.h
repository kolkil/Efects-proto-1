#pragma once

template <typename T>
class IEffect
{
public:
    virtual void apply(T* buffer, size_t size) = 0;
};
