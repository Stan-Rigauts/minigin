#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include "SpriteAnimatorComponent.h"
#include "servicelocator.h"
#include "SoundIds.h"
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
        void UpdateAnimation();
        void Move(float dirX, float dirY);
        void SetGrid(GridComponent* grid) { m_Grid = grid; }
        void SetGameManager(GameManagerComponent* manager){m_GameManager = manager;}
        void SetFrozen(bool f) { m_Frozen = f; }
        void FreezeFor(float seconds);
        void SetAnimator(dae::SpriteAnimatorComponent* animator) { m_pAnimator = animator; }

    private:
        bool IsAlignedToGrid() const;
        bool CanMove(float dirX, float dirY) const;
        void SnapToGrid();
        void ApplyTeleport();

        dae::SpriteAnimatorComponent* m_pAnimator = nullptr;


        bool m_Frozen{ false };
        float m_FreezeTimer{ 0.f };
        float m_Speed{};
        float m_Delta{};
        glm::vec2 m_CurrentDir{ 0.f, 0.f };
        glm::vec2 m_QueuedDir{ 0.f, 0.f };
        GridComponent* m_Grid{ nullptr };
        GameManagerComponent* m_GameManager{};
        float m_WakkaTimer{ 0.f };
        static constexpr float WAKKA_INTERVAL{ 0.18f };
    };
}