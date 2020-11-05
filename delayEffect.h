#pragma once
#include "IEffect.h"

#include <vector>
#include <iostream>

class delayEffect :
    public IEffect
{
private:
    std::vector<std::unique_ptr<char[]>> buffers;
    size_t single_buffer_size = 0;
    size_t nbuffers = 0;
    int out_buffer = 0;
    int in_buffer = 0;
    bool ready_flag = false;

public:
    delayEffect(size_t nbuffers, size_t single_buffer_size);
    void apply(char* buffer, size_t len);
};

