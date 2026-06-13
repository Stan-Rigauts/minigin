#include "HealthComponent.h"
#include "GameEvent.h" 

namespace dae
{
    void HealthComponent::Damage(int amount)
    {
        if (m_Lives <= 0)
        {
            Die();
        }
        else
        {
            m_Lives -= amount;
            m_Subject.Notify(static_cast<int>(GameEvent::PlayerDamaged), &GetOwner());
        }
    }

    void HealthComponent::Die()
    {
        m_Subject.Notify(static_cast<int>(GameEvent::PlayerDied), &GetOwner());
    }
}