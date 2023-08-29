#include "buffer-manager.h"

void BufferManager::clearReadBuffer()
{
    memset(BufferManager::READ_BUFFER, 0, sizeof(BufferManager::READ_BUFFER));
}

void BufferManager::clearWriteBuffer()
{
    memset(BufferManager::WRITE_BUFFER, 0, sizeof(BufferManager::WRITE_BUFFER));
}

size_t BufferManager::getReadBufferLength()
{
    return (size_t)READ_BUFFER_SIZE;
}
size_t BufferManager::getWriteBufferLength()
{
    return (size_t)WRITE_BUFFER_SIZE;
}