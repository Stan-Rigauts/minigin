#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace dae { class GridComponent; 
                class GameManagerComponent;
               }

namespace dae
{
    class MoveComponent final : public Component
    {
    public:
        MoveComponent(GameObject& owner, float speed);
        ~MoveComponent() override { m_Grid = nullptr; }
        void Update(float delta) override;
        void Move(float dirX, float dirY);
        void SetGrid(GridComponent* grid) { m_Grid = grid; }
        void SetGameManager(GameManagerComponent* manager){m_GameManager = manager;}

    private:
        bool IsAlignedToGrid() const;
        bool CanMove(float dirX, float dirY) const;
        void SnapToGrid();

        float m_Speed{};
        float m_Delta{};
        glm::vec2 m_CurrentDir{ 0.f, 0.f };
        glm::vec2 m_QueuedDir{ 0.f, 0.f };
        GridComponent* m_Grid{ nullptr };
        GameManagerComponent* m_GameManager{};

    };
}