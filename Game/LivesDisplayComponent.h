#pragma once
#include "Component.h"
#include "Observer.h"
#include "TextComponent.h"
#include "HealthComponent.h"

namespace dae
{
    class LivesDisplayComponent final : public Component, public GameObserver
    {
    public:
        LivesDisplayComponent(GameObject& owner, HealthComponent& healthComp, TextComponent& textComp)
            : Component(owner)
            , m_HealthComponent(healthComp)
            , m_TextComponent(textComp)
        {
            m_HealthComponent.GetSubject().AddObserver(this);
        }
        ~LivesDisplayComponent() override
        {
            m_HealthComponent.GetSubject().RemoveObserver(this);
        }
        void OnNotify(GameEvent event, dae::GameObject* owner) override
        {
            if (event == GameEvent::PlayerDamaged || event == GameEvent::PlayerDied)
            {
                auto lives = owner->GetComponent<HealthComponent>()->GetLives();
                m_TextComponent.SetText("Lives: " + std::to_string(lives));
            }
        }
    private:
        HealthComponent& m_HealthComponent;
        TextComponent& m_TextComponent;
    };
}