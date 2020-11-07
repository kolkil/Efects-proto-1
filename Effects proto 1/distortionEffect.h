#pragma once
#include "IEffect.h"

#include <limits>

template <typename T>
class distortionEffect :
    public IEffect<T>
{
private:
    double threshold;
    T maxT = (std::numeric_limits<T>::max)();
public:
    distortionEffect(double threshold);
    void apply(T* buffer, size_t len);
};

template<typename T>
inline distortionEffect<T>::distortionEffect(double threshold)
{
    this->threshold = threshold;
}

template<typename T>
inline void distortionEffect<T>::apply(T* buffer, size_t len)
{
    T threshold_value = threshold * maxT;
    for (int i = 0; i < len; ++i)
    {
        if (buffer[i] > threshold_value)
        {
            buffer[i] = threshold_value;
        }
        else if (buffer[i] < -threshold_value)
        {
            buffer[i] = -threshold_value;
        }
    }
}
