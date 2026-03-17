#pragma once
#include "Component.h"
#include "Subject.h"

namespace dae
{
    class HealthComponent final : public Component
    {
    public:
        HealthComponent(GameObject& owner, int lives = 3)
            : Component(owner)
            , m_Lives(lives)
        {
        }

        void Damage(int amount = 1);
        void Die();

        int GetLives() const { return m_Lives; }
        Subject& GetSubject() { return m_Subject; }

    private:
        int m_Lives{};
        Subject m_Subject{};
    };
}