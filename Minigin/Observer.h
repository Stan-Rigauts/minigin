#pragma once
#include "GameEvent.h"

namespace dae { class GameObject; }


class Observer
{
public:
    virtual ~Observer() = default;
    virtual void OnNotify(GameEvent event, dae::GameObject* owner) = 0;
};