#include "seria.h"

XSpace::XSpace() : occupied_size_(0), left_size_(CMD_BUF_SIZE), unser_idx_(0)
{
    memset(buf_, 0, CMD_BUF_SIZE);
}

XSpace::XSpace(void* buf, int len) : occupied_size_(0), left_size_(CMD_BUF_SIZE), unser_idx_(0)
{
    memcpy(buf_, buf, len);
}

void XSpace::Trim()
{
    for (int i = 0; i < CMD_BUF_SIZE; ++i) {
        if (buf_[i] == 0x0) {
            buf_[i] = 0x3F; //'?'
        }
    }

    buf_[occupied_size_] = 0x0;
}

void XSpace::Untrim()
{
    for (size_t i = 0; i < CMD_BUF_SIZE; ++i) {
        if (buf_[i] == 0x3F) {
            buf_[i] = 0x0;
        }
    }
}
