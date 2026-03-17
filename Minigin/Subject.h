#pragma once
#include <vector>
#include "Observer.h"

class Subject
{
public:
    ~Subject();
    void AddObserver(Observer* observer);
    void RemoveObserver(Observer* observer);
    void Notify(GameEvent event, int value);

private:
    std::vector<Observer*> m_Observers;
};
