#pragma once
#include <vector>
#include "Observer.h"
namespace dae { class GameObject; }

class Subject
{
public:
    ~Subject();
    void AddObserver(Observer* observer);
    void RemoveObserver(Observer* observer);
    void Notify(GameEvent event, dae::GameObject* owner);
    bool IsDestroyed() const { return m_Destroyed; }
private:
    std::vector<Observer*> m_Observers;
    bool m_Destroyed{ false };
};