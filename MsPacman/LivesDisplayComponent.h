#pragma once
#include "Component.h"
#include "Observer.h"
#include "HealthComponent.h"
#include "RenderComponent.h"
#include "Scene.h"
#include <vector>
#include <string>

namespace dae
{
    class LivesDisplayComponent final : public Component, public GameObserver
    {
    public:
        LivesDisplayComponent(GameObject& owner, HealthComponent& healthComp,
            Scene* scene, const std::string& iconTexture,
            float iconSize, float startX, float startY)
            : Component(owner)
            , m_HealthComponent(healthComp)
            , m_pScene(scene)
            , m_IconTexture(iconTexture)
            , m_IconSize(iconSize)
            , m_StartX(startX)
            , m_StartY(startY)
        {
            m_HealthComponent.GetSubject().AddObserver(this);
            RebuildIcons(m_HealthComponent.GetLives()); 

        }

        ~LivesDisplayComponent() override
        {
            m_HealthComponent.GetSubject().RemoveObserver(this);
        }

       
        void OnNotify(GameEvent event, dae::GameObject*) override
        {
            if (event == GameEvent::PlayerDamaged || event == GameEvent::PlayerDied)
                RebuildIcons(m_HealthComponent.GetLives());
        }

    private:
        void RebuildIcons(int lives)
        {
            for (auto* go : m_Icons)
                go->MarkForDestroy();
            m_Icons.clear();

            for (int i = 0; i < lives; ++i)
            {
                auto go = std::make_unique<GameObject>();
                go->SetTag("HUD");
                go->SetLocalPosition(m_StartX + i * (m_IconSize + 4.f), m_StartY);

                auto render = std::make_unique<RenderComponent>(*go);
                render->SetTexture(m_IconTexture);
                render->SetSourceRect(0, 0, 16, 16);
                render->SetSize(m_IconSize, m_IconSize);
                go->AddComponent(std::move(render));

                m_Icons.push_back(go.get());
                m_pScene->Add(std::move(go));
            }
        }

        HealthComponent& m_HealthComponent;
        Scene* m_pScene;
        std::string m_IconTexture;
        float m_IconSize;
        float m_StartX;
        float m_StartY;
        std::vector<GameObject*> m_Icons;
    };
}