#ifndef QTDEMO_NOTIFY_THREAD_H_
#define QTDEMO_NOTIFY_THREAD_H_

#include <QThread.h>

class NotifyThread : public QThread {
    Q_OBJECT

protected:
    void run();

};

#endif //QTDEMO_NOTIFY_THREAD_H_
