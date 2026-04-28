#pragma once
#include "Component.h"
#include <vector>

namespace dae
{
    class GameManagerComponent;
    class GameObject;

    class PelletComponent final : public Component
    {
    public:
        PelletComponent(GameObject& owner, GameManagerComponent* manager,
            std::vector<GameObject*> players,
            bool isPowerPellet = false,
            float pickupRadius = 8.f);

        void Update(float delta) override;

        bool IsPowerPellet() const { return m_IsPowerPellet; }

    private:
        GameManagerComponent* m_GameManager{};
        std::vector<GameObject*> m_Players{};
        bool                     m_IsPowerPellet{};
        float                    m_PickupRadius{};
        bool                     m_Collected{ false };
    };
}