#include "HealthComponent.h"

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
            m_Subject.Notify(GameEvent::PlayerDamaged, &GetOwner());
        }
    }
    void HealthComponent::Die()
    {
        m_Subject.Notify(GameEvent::PlayerDied, &GetOwner());
    }
}