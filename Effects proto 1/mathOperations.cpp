#include "mathOperations.h"

#include <cmath>
#include <algorithm>

#define M_PI       3.14159265358979323846

void mathOperations::fillHammingValues()
{
    for (size_t i = 0; i < buffer_len; ++i)
    {
        hamming_values[i] = 0.53836 - 0.46164 * cos(((2 * M_PI * ((double)i))) / ((double)(buffer_len - 1)));
    }
}

mathOperations::mathOperations(size_t buffer_len)
{
    this->buffer_len = buffer_len;
    this->nextpow2 = nextPowerOf2(buffer_len);
    hamming_values = std::make_unique<double[]>(nextpow2);
    data_buffer = std::make_unique<double[]>(2 * nextpow2);
    fillHammingValues();
}

double mathOperations::getMax(double* data)
{
    double maxi = (std::numeric_limits<double>::min)();
    for (int i = 0; i < buffer_len; ++i)
    {
        maxi = (std::max)(maxi, data[i]);
    }
    return maxi;
}

int mathOperations::nextPowerOf2(int a)
{
    int i = 1;
    for (; i < a; i <<= 1);
    return i;
}

void mathOperations::four1(double* data)
{
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    // reverse-binary reindexing
    n = nextpow2 << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
        if (j > i) {
            std::swap(data[j - 1], data[i - 1]);
            std::swap(data[j], data[i]);
        }
        m = nextpow2;
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

void mathOperations::applyHammingWindow(double* data)
{
    for (size_t i = 0; i < buffer_len; ++i)
    {
        data[i] *= hamming_values[i];
    }
}
