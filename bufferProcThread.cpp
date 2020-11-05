#include "bufferProcThread.h"
#include <mutex>
#include <thread>

void bufferProcThread::start()
{
}

void bufferProcThread::work()
{
    //char* out_buffer = new char[buffer_len];

    while (1)
    {
        std::mutex mtx;
        std::unique_lock<std::mutex> ulck(mtx);
        cnd.wait(ulck);
        memcpy(buffer, waveIn->getCurrentBuffer(), buffer_len);

        for (int i = 0; i < effects.size(); ++i)
        {
            effects[i]->apply(buffer, buffer_len);
        }

        if (outwriter)
        {
            outwriter->apply(buffer, buffer_len);
        }

        ulck.unlock();
    }
}
