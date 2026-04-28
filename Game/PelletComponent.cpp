#include "PelletComponent.h"
#include "GameObject.h"
#include "GameManagerComponent.h"
#include "Transform.h"
#include <cmath>

namespace dae
{
    PelletComponent::PelletComponent(GameObject& owner,
        GameManagerComponent* manager,
        std::vector<GameObject*> players,
        bool isPowerPellet,
        float pickupRadius)
        : Component(owner)
        , m_GameManager(manager)
        , m_Players(std::move(players))
        , m_IsPowerPellet(isPowerPellet)
        , m_PickupRadius(pickupRadius)
    {
    }

    void PelletComponent::Update(float /*delta*/)
    {
        if (m_Collected) return;

        const auto myPos = GetOwner().GetTransform().GetWorldPosition();

        for (auto* player : m_Players)
        {
            if (!player || player->IsMarkedForDestroy()) continue;

            const auto pPos = player->GetTransform().GetWorldPosition();
            const float dx = myPos.x - pPos.x;
            const float dy = myPos.y - pPos.y;

            if (std::sqrt(dx * dx + dy * dy) < m_PickupRadius)
            {
                m_Collected = true;
                GetOwner().MarkForDestroy();

                if (m_GameManager)
                    m_GameManager->PelletCollected();

                return;
            }
        }
    }
}