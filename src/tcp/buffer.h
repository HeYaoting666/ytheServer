#pragma once
#include "algorithm"
#include "../log/logger.h"

namespace ythe {

// ****已读数据**** mReadIndex ****写入未读数据**** mWriteIndex ****剩余可写数据****

class TCPBuffer {
public:
    typedef std::shared_ptr<TCPBuffer> sp;

private:
    int mReadIndex  = 0;

    int mWriteIndex = 0;

public:
    std::vector<char> mBuffer;

public:
    explicit TCPBuffer(int size);

public:
    // 返回可读取字节数
    int ReadAble() const { return mWriteIndex - mReadIndex; }

    // 返回可写入的字节数
    int WriteAble() const { return mBuffer.size() - mWriteIndex; }

    // 获取缓冲区指针
    char* Data() { return mBuffer.data(); }

    // 返回缓冲区容量
    int Size() const { return mBuffer.size(); }

    int ReadIndex() const { return mReadIndex; }

    int WriteIndex() const { return mWriteIndex; }

    void WriteToBuffer(const char* buf, int size);

    void ReadFromBuffer(std::vector<char>& result, int size);

    void MoveReadIndex(int size);

    void MoveWriteIndex(int size);

    // 根据给定值调整缓冲区容量
    void ResizeBuffer(int newSize);

    char& operator[](int index);

    const char& operator[](int index) const;

private:
    // 已读度数据超过缓冲区容量的 1/3 时，调整缓冲区容量
    void adjustBuffer();

};

}