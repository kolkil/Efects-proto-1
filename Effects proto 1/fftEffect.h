#pragma once
#include "IEffect.h"
#include "mathOperations.h"

template <typename T>
class fftEffect :
    public IEffect<T>
{
private:
    mathOperations math_ops;

public:
    fftEffect(size_t buffer_size);
    void apply(T* buffer, size_t buffer_len);
};

template<typename T>
inline fftEffect<T>::fftEffect(size_t buffer_size)
    : math_ops(buffer_size) { }

template<typename T>
inline void fftEffect<T>::apply(T* buffer, size_t buffer_len)
{
    math_ops.applyHammingFourierTakeAbsAndScale(buffer);
}

