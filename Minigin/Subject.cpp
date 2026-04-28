#include "Subject.h"
#include <algorithm>

Subject::~Subject()
{
    m_Destroyed = true;
    m_Observers.clear();
}

void Subject::AddObserver(Observer* observer)
{
    m_Observers.push_back(observer);
}

void Subject::RemoveObserver(Observer* observer)
{
    if (m_Destroyed) return;
    m_Observers.erase(std::remove(m_Observers.begin(), m_Observers.end(), observer), m_Observers.end());
}

void Subject::Notify(int event, dae::GameObject* owner)
{
    for (auto* observer : m_Observers)
        observer->OnNotify(event, owner);
}