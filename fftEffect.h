#pragma once
#include "IEffect.h"

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#define M_PI       3.14159265358979323846
template <typename T>
class fftEffect :
    public IEffect<T>
{
private:
    size_t nextpow2;
    size_t buffer_len;
    double scale_factor = 0.00001;
    double display_h = 1440;
    std::unique_ptr<double[]> data_array;
    std::unique_ptr<double[]> humming_values;

    static inline int nextPowerOf2(int a);
    void four1(double* data, unsigned long nn);
    void applyHummingWindow();
    void applyLog();
    inline double getMax();
    void applyScaleFactor();

public:
    fftEffect(size_t buffer_size);
    void apply(T* buffer, size_t buffer_len);
};

template<typename T>
inline fftEffect<T>::fftEffect(size_t buffer_size)
{
    this->nextpow2 = nextPowerOf2(buffer_size);
    data_array = std::make_unique<double[]>(nextpow2 * 2);
    humming_values = std::make_unique<double[]>(buffer_size);

    for (int i = 0; i < buffer_size; ++i)
    {
        humming_values[i] = 0.53836 - 0.46164 * cos(((2 * M_PI * ((double)i))) / ((double)(buffer_size - 1)));
    }
}

template<typename T>
inline void fftEffect<T>::apply(T* buffer, size_t buffer_len)
{
    //memset(im_buffer.get(), 0, sizeof(double) * nextpow2);
    //memset(re_buffer.get(), 0, sizeof(double) * nextpow2);
    this->buffer_len = buffer_len;
    memset(data_array.get(), 0, sizeof(double) * nextpow2 * 2);
    applyHummingWindow();
    /* for (int i = 0; i < buffer_len; ++i)
     {
         re_buffer[i] = buffer[i];
     }

     FFT(1, nextpow2, re_buffer.get(), im_buffer.get());*/

    for (int i = 0; i < buffer_len; ++i)
    {
        data_array[2 * i] = buffer[i];
    }

    four1(data_array.get(), nextpow2);

    for (int i = 0; i < buffer_len; ++i)
    {
        data_array[i] = sqrt(data_array[i] * data_array[i] + data_array[i + 1] * data_array[i + 1]);
    }

    //applyLog();
    applyScaleFactor();

    for (int i = 0; i < buffer_len; ++i)
    {
        buffer[i] = data_array[i];
    }
}

template<typename T>
inline int fftEffect<T>::nextPowerOf2(int a)
{
    int i = 1;
    for (; i <= a; i <<= 1);
    return i;
}

template<typename T>
inline void fftEffect<T>::four1(double* data, unsigned long nn)
{
    {
        unsigned long n, mmax, m, j, istep, i;
        double wtemp, wr, wpr, wpi, wi, theta;
        double tempr, tempi;

        // reverse-binary reindexing
        n = nn << 1;
        j = 1;
        for (i = 1; i < n; i += 2) {
            if (j > i) {
                std::swap(data[j - 1], data[i - 1]);
                std::swap(data[j], data[i]);
            }
            m = nn;
            while (m >= 2 && j > m) {
                j -= m;
                m >>= 1;
            }
            j += m;
        };

        // here begins the Danielson-Lanczos section
        mmax = 2;
        while (n > mmax) {
            istep = mmax << 1;
            theta = -(2 * M_PI / mmax);
            wtemp = sin(0.5 * theta);
            wpr = -2.0 * wtemp * wtemp;
            wpi = sin(theta);
            wr = 1.0;
            wi = 0.0;
            for (m = 1; m < mmax; m += 2) {
                for (i = m; i <= n; i += istep) {
                    j = i + mmax;
                    tempr = wr * data[j - 1] - wi * data[j];
                    tempi = wr * data[j] + wi * data[j - 1];

                    data[j - 1] = data[i - 1] - tempr;
                    data[j] = data[i] - tempi;
                    data[i - 1] += tempr;
                    data[i] += tempi;
                }
                wtemp = wr;
                wr += wr * wpr - wi * wpi;
                wi += wi * wpr + wtemp * wpi;
            }
            mmax = istep;
        }
    }
}

template<typename T>
inline void fftEffect<T>::applyHummingWindow()
{
    for (int i = 0; i < buffer_len; ++i)
    {
        data_array[i] *= humming_values[i];
    }
}

template<typename T>
inline void fftEffect<T>::applyLog()
{
    for (int i = 0; i < buffer_len; ++i)
    {
        data_array[i] = 20 * log10(data_array[i]);
    }
}

template<typename T>
inline double fftEffect<T>::getMax()
{
    double maxi = (std::numeric_limits<double>::min)();
    for (int i = 0; i < buffer_len; ++i)
    {
        maxi = (std::max)(maxi, data_array[i]);
    }
    return maxi;
}

template<typename T>
inline void fftEffect<T>::applyScaleFactor()
{
    double maxi = getMax();
    for (int i = 0; i < buffer_len; ++i)
    {
        data_array[i] /= maxi;
        data_array[i] *= display_h;
    }
}
