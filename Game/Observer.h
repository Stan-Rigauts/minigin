#pragma once
#include "Observer.h"
#include "GameEvent.h"
namespace dae { class GameObject; }

class GameObserver : public Observer
{
public:
    void OnNotify(int event, dae::GameObject* owner) override final
    {
        OnNotify(static_cast<GameEvent>(event), owner);
    }
    virtual void OnNotify(GameEvent event, dae::GameObject* owner) = 0;
};