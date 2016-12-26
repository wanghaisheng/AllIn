#ifndef SERAILIZATION_H_
#define SERAILIZATION_H_

#include <iostream>
#include "common_definitions.h"

#ifdef _DEBUG
#define PRIVATE public
#else
#define PRIVATE private
#endif

class XSpace {

public:
    XSpace();

    XSpace(void* buf, int len);

    // 重载<<运算符, 序列化
    template <typename T>
    friend XSpace& operator<<(XSpace& xs, const T& t)
    {
        int len = sizeof(T);
        memcpy(xs.buf_ + xs.occupied_size_, &t, len);
        xs.occupied_size_ += len;
        xs.left_size_ = CMD_BUF_SIZE - xs.occupied_size_;

        return xs;
    }

    void Trim();

    // 在反序列化前调用
    void Untrim();

    // 重载>>运算符, 反序列化
    template <typename T>
    friend XSpace& operator>>(XSpace& xs, T& t)
    {
        int len = sizeof(T);
        memcpy(&t, xs.buf_ + xs.unser_idx_, len);
        xs.unser_idx_ += len;

        return xs;
    }

    char* GetBuf()
    {
        return buf_;
    }

    int GetSize()
    {
        return sizeof(buf_);
    }

public:
    char buf_[CMD_BUF_SIZE];
    int occupied_size_;
    int left_size_;

    int unser_idx_;
};

#endif //SERAILIZATION_H_
