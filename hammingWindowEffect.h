#pragma once
#include "IEffect.h"

#include <iostream>

template <typename T>
class hammingWindowEffect :
    public IEffect<T>
{
private:
    std::unique_ptr<double[]> window_values;

    //static double 
};

