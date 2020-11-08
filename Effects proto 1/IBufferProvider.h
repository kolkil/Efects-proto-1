#pragma once

template <typename T>
class IBufferProvider
{
public:
    virtual void waitForBuffer() = 0;
    virtual void start() = 0;
    virtual T* getBuffer() = 0;

    /* return buffer size in elements of type T */
    virtual size_t getBufferLen() = 0;

    /* return buffer size in bytes */
    virtual size_t getBufferSize() = 0;
};
