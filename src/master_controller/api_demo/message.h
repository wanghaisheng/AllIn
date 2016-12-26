#ifndef QTDEMO_MESSAGE_H_
#define QTDEMO_MESSAGE_H_

#include <iostream>

#define CMD_OPEN_DEV 100
#define CMD_CLOSE_DEV 101
#define CMD_UPDATE_PROGRESS 102

class Message {
    enum MAX_LEN {
        MAX_DATA = 1024
    };

public:
    Message(int what) : what_(what) 
    {
        memset(data_, 0, MAX_DATA);
    }

public:
    int what_;

    int err_;

    int obj_;

    int val_;

    std::string msg_;

    unsigned char data_[MAX_DATA];
};

#endif //QTDEMO_MESSAGE_H_
