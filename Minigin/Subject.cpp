#include "Subject.h"
#include <algorithm>

Subject::~Subject()
{
    m_Observers.clear();
}

void Subject::AddObserver(Observer* observer)
{
    if (observer && std::find(m_Observers.begin(), m_Observers.end(), observer) == m_Observers.end())
    {
        m_Observers.push_back(observer);
    }
}

void Subject::RemoveObserver(Observer* observer)
{
    if (m_Observers.empty())
        return;

    auto it = std::find(m_Observers.begin(), m_Observers.end(), observer);
    if (it != m_Observers.end())
        m_Observers.erase(it);
}


void Subject::Notify(GameEvent event, int value)
{
    for (auto* observer : m_Observers)
    {
        if (observer)
            observer->OnNotify(event, value);
    }
}
