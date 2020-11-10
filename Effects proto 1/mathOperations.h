#pragma once

#include <array>
#include <iostream>
#include <algorithm>

class mathOperations
{
private:
    size_t buffer_len;
    size_t nextpow2;
    std::unique_ptr<double[]> hamming_values;
    std::unique_ptr<double[]> data_buffer; // this is pre-allocated buffer for operations, length = 2*nextpow2

    void fillHammingValues();

public:
    mathOperations(size_t buffer_len);
    double getMax(double* data);
    int nextPowerOf2(int a);
    void four1(double* data);
    void applyHammingWindow(double* data);

    template <typename T>
    void applyHammingFourierTakeAbsAndScale(T* buffer);

    template <typename T>
    T getMaxAbs(T* buffer);

    template <typename T>
    static T getMaxAbs(T* buffer, size_t len);
};

template<typename T>
inline void mathOperations::applyHammingFourierTakeAbsAndScale(T* buffer)
{
    double max_val = 0;
    T max_T_val = (std::numeric_limits<T>::max)();
    memset(data_buffer.get(), 0, sizeof(double) * nextpow2 * 2);

    // apply Hamming Window
    for (size_t i = 0; i < buffer_len; ++i)
    {
        data_buffer[2 * i] = hamming_values[i] * buffer[i];
    }

    four1(data_buffer.get());

    // take abs
    for (size_t i = 0; i < buffer_len; ++i)
    {
        data_buffer[i] = sqrt(data_buffer[i] * data_buffer[i] + data_buffer[i + 1] * data_buffer[i + 1]);
    }

    // scale and pass to buffer
    max_val = getMax(data_buffer.get());
    for (size_t i = 0; i < buffer_len; ++i)
    {
        data_buffer[i] /= max_val;
        data_buffer[i] *= max_T_val;
        buffer[i] = data_buffer[i];
    }
}

template<typename T>
inline T mathOperations::getMaxAbs(T* buffer)
{
    return getMaxAbs(buffer, buffer_len);
}

template<typename T>
inline T mathOperations::getMaxAbs(T* buffer, size_t len)
{
    T maxi = (std::numeric_limits<T>::min)();

    for (size_t i = 0; i < len; ++i)
    {
        maxi = (std::max)(maxi, (T)std::abs(buffer[i]));
    }

    return maxi;
}
