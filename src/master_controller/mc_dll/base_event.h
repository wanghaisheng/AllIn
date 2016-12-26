#ifndef CONTROLLER_BASE_EVENT_H_
#define CONTROLLER_BASE_EVENT_H_

#include <string>
#include "common_definitions.h"

namespace MC {

class BaseEvent {

public:
    explicit BaseEvent(std::string des);

    virtual ~BaseEvent();

    virtual void Execute();

    virtual void SpecificExecute() = 0;

public:
    ErrorCode exception_;

private:
    std::string des_;
};

}

#endif // CONTROLLER_BASE_EVENT_H_
