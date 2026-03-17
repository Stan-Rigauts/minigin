#pragma once
#include "GameEvent.h"
class Observer
{
public:
    virtual ~Observer() = default;
    virtual void OnNotify(GameEvent event, int value) = 0;
};