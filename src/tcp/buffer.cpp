#include "buffer.h"
#include "src/log/logger.h"
#include <algorithm>

namespace ythe {

TCPBuffer::TCPBuffer(int size)
{
    mBuffer.resize(size);
}

void TCPBuffer::WriteToBuffer(const char* buf, int writeSize)
{
    if(writeSize > WriteAble()) {
        int newSize = 1.5 * (mWriteIndex + writeSize);
        ResizeBuffer(newSize);
    }
    std::copy_n(buf, writeSize, mBuffer.begin() + mWriteIndex);
    MoveWriteIndex(writeSize);
}

void TCPBuffer::ReadFromBuffer(std::vector<char>& result, int size)
{
    if(ReadAble() == 0)
        return;
    
    int readCount = std::min(ReadAble(), size);
    result.resize(readCount);
    std::copy_n(mBuffer.begin() + mReadIndex, readCount, result.begin());
    MoveReadIndex(readCount);
}

void TCPBuffer::MoveReadIndex(int size) {
    if(mReadIndex + size > mWriteIndex) {
        ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, old_write_index %d", size, mReadIndex, mWriteIndex)
        return;
    }
    mReadIndex += size;
    adjustBuffer();
}

void TCPBuffer::MoveWriteIndex(int size) {
    if(mWriteIndex + size > mBuffer.size()) {
        ERRORLOG("moveReadIndex error, invalid size %d, old_write_index %d, old_buffer_size %d", size, mWriteIndex, mBuffer.size())
        return;
    }
    mWriteIndex += size;
    adjustBuffer();
}

void TCPBuffer::ResizeBuffer(int newSize)
{
    if(newSize < mBuffer.size())
        return;
    std::vector<char> tmp(newSize);
    int cpCount = ReadAble(); // 需要拷贝的字节数
    std::copy_n(mBuffer.begin() + mReadIndex, cpCount, tmp.begin());

    // 重新设置缓冲区
    mBuffer.swap(tmp);
    mReadIndex = 0;
    mWriteIndex = mReadIndex + cpCount;
}

char& TCPBuffer::operator[](int index) {
    if(index >= mWriteIndex)
        throw std::out_of_range("subscript out of range");
    return mBuffer[index];
}

const char& TCPBuffer::operator[](int index) const {
    if(index >= mWriteIndex)
        throw std::out_of_range("subscript out of range");
    return mBuffer[index];
}

void TCPBuffer::adjustBuffer()
{
    if(mReadIndex < (mBuffer.size() / 2))
        return;
    std::vector<char> tmp(mBuffer.size());
    int cpCount = ReadAble();
    std::copy_n(mBuffer.begin() + mReadIndex, cpCount, tmp.begin());

    // 重新设置缓冲区
    mBuffer.swap(tmp);
    mReadIndex = 0;
    mWriteIndex = mReadIndex + cpCount;
}

}