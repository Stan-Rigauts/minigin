#pragma once
#include "Component.h"
#include "Observer.h"
#include "TextComponent.h"
#include "ScoreComponent.h"

namespace dae
{
    class ScoreDisplayComponent final : public Component, public Observer
    {
    public:
        ScoreDisplayComponent(GameObject& owner, ScoreComponent& scoreComp, TextComponent& textComp)
            : Component(owner)
            , m_ScoreComponent(scoreComp)
            , m_TextComponent(textComp)
        {
            m_ScoreComponent.GetSubject().AddObserver(this);
        }

        ~ScoreDisplayComponent() override
        {
            m_ScoreComponent.GetSubject().RemoveObserver(this);
        }

        void OnNotify(GameEvent event, int value) override
        {
            if (event == GameEvent::ScoreChanged || event == GameEvent::ScoreReset)
            {
                UpdateText(value);
            }
        }

    private:
        void UpdateText(int score)
        {
            m_TextComponent.SetText("Score: " + std::to_string(score));
        }

        ScoreComponent& m_ScoreComponent;
        TextComponent& m_TextComponent;
    };
}