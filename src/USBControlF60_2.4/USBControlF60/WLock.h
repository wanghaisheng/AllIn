#ifndef  _H_WLOCK_
#define  _H_WLOCK_
#include <windows.h>


class IMyLock

{

public:

    virtual ~IMyLock() {}

 

    virtual void Lock() const = 0;

    virtual void Unlock() const = 0;

};

 

//ª•≥‚∂‘œÛÀ¯¿‡

class Mutex : public IMyLock

{

public:

    Mutex();

    ~Mutex();

 

    virtual void Lock() const;

    virtual void Unlock() const;

 

private:

    HANDLE m_mutex;

};


//À¯

class CLock

{

public:

    CLock(const IMyLock&);

    ~CLock();

 

private:

    const IMyLock& m_lock;

};

 

#endif