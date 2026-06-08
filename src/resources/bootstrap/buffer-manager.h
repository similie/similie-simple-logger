#include "Particle.h"
#ifndef system_store_buffers_h
#define system_store_buffers_h

#define READ_BUFFER_SIZE 1024
#define WRITE_BUFFER_SIZE 512
#define FLOAT_BUFFER_MAX_LENGTH 20
#define FLOAT_BUFFER_MAX_HEIGHT 25
class BufferManager
{
private:
public:
    inline static char READ_BUFFER[READ_BUFFER_SIZE];
    inline static char WRITE_BUFFER[WRITE_BUFFER_SIZE];
    // inline static float VALUES_BUFFER[FLOAT_BUFFER_MAX_HEIGHT][FLOAT_BUFFER_MAX_LENGTH];
    static void clearWriteBuffer();
    static void clearReadBuffer();
    static size_t getReadBufferLength();
    static size_t getWriteBufferLength();
};

#endif