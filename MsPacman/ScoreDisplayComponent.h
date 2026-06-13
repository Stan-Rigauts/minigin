#pragma once
#include "Component.h"
#include "ScoreComponent.h"
#include "Observer.h"
#include "RenderComponent.h"
#include "Scene.h"
#include <vector>
#include <string>

namespace dae
{
    class ScoreDisplayComponent final : public Component, public GameObserver
    {
    public:
        ScoreDisplayComponent(GameObject& owner, ScoreComponent& scoreComp,
            Scene* scene, const char* tag,
            float startX, float startY, float digitSize)
            : Component(owner)
            , m_ScoreComponent(&scoreComp)
            , m_pScene(scene)
            , m_Tag(tag)
            , m_StartX(startX)
            , m_StartY(startY)
            , m_DigitSize(digitSize)
        {
            m_pSubject = &m_ScoreComponent->GetSubject();
            m_pSubject->AddObserver(this);
            RebuildDigits(m_ScoreComponent->GetScore());
        }

        ScoreDisplayComponent(GameObject& owner, int staticValue,
            Scene* scene, const char* tag,
            float startX, float startY, float digitSize)
            : Component(owner)
            , m_UseStaticValue(true)
            , m_StaticValue(staticValue)
            , m_pScene(scene)
            , m_Tag(tag)
            , m_StartX(startX)
            , m_StartY(startY)
            , m_DigitSize(digitSize)
        {
            RebuildDigits(staticValue);
        }

        ~ScoreDisplayComponent() override
        {
            if (!m_UseStaticValue && m_pSubject)
                m_pSubject->RemoveObserver(this);
        }

        void OnNotify(GameEvent event, dae::GameObject*) override
        {
            if (m_UseStaticValue)
                return;

            if (event == GameEvent::ScoreChanged || event == GameEvent::ScoreReset)
                RebuildDigits(m_ScoreComponent->GetScore());
        }

        void SetStaticValue(int value)
        {
            if (!m_UseStaticValue)
                return;

            m_StaticValue = value;
            RebuildDigits(value);
        }

        void OnSubjectDestroyed() override { m_pSubject = nullptr; }
    private:
        void RebuildDigits(int score)
        {
            if (m_UseStaticValue)
                score = m_StaticValue;

            for (auto* go : m_Digits)
                go->MarkForDestroy();
            m_Digits.clear();

            std::string scoreString = std::to_string(score);
            while (scoreString.size() < 6) scoreString = "0" + scoreString;

            for (int index = 0; index < static_cast<int>(scoreString.size()); ++index)
            {
                int digit = scoreString[index] - '0';

                auto go = std::make_unique<GameObject>();
                go->SetTag(m_Tag);
                go->SetLocalPosition(m_StartX + index * m_DigitSize, m_StartY);

                auto render = std::make_unique<RenderComponent>(*go);
                render->SetTexture("Numbers.png");
                render->SetSourceRect(digit * 18, 0, 16, 16);
                render->SetSize(m_DigitSize, m_DigitSize);
                render->SetLayer(6);
                go->AddComponent(std::move(render));

                m_Digits.push_back(go.get());
                m_pScene->Add(std::move(go));
            }
        }

        ScoreComponent* m_ScoreComponent = nullptr;
        bool m_UseStaticValue = false;
        int m_StaticValue = 0;

        Scene* m_pScene;
        const char* m_Tag;
        float m_StartX, m_StartY, m_DigitSize;
        std::vector<GameObject*> m_Digits;
        Subject* m_pSubject = nullptr;

    };

}