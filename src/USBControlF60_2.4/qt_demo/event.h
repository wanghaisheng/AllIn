#ifndef QTDEMO_EVETN_H_
#define QTDEMO_EVETN_H_

class QMainWindow;

class Event {
public:
    virtual void Execute() = 0;
};

class OpenEventS: public Event {
public:
    OpenEventS(QMainWindow* win) : win_(win) {}

    virtual void Execute();

private:
    QMainWindow* win_;
};

class CloseEvent : public Event {
public:
    CloseEvent(QMainWindow* win) : win_(win) {}

    virtual void Execute();

private:
    QMainWindow* win_;
};

class EnableFactoryEvent : public Event {
public:
    EnableFactoryEvent(QMainWindow* win) : win_(win) {}



private:
    QMainWindow* win_;
};
#endif //QTDEMO_EVETN_H_
