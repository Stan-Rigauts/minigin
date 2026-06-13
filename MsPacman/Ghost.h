#pragma once
#include "Component.h"
#include "GhostState.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "GameObject.h"
#include "SpriteAnimatorComponent.h"

namespace dae
{
    class GridComponent;
    class RenderComponent;
}


class Ghost final : public dae::Component
{
public:
    enum class Type { Blinky, Pinky, Inky, Clyde };

    explicit Ghost(dae::GameObject& owner);
    ~Ghost() override = default;

    void Update(float deltaTime) override;
    void UpdateAnimation();
    void UpdatePlayerControlled(float deltaTime);
    void Start();

    void SetGrid(dae::GridComponent* grid) { m_pGrid = grid; }
    void SetType(Type type) { m_Type = type; }
    void SetExitPath(std::vector<glm::vec2> path) { m_ExitPath = std::move(path); }


    void SetPacManPosition(glm::vec2 pos) { m_PacManPosition = pos; }
    void SetBlinkyPosition(glm::vec2 pos) { m_BlinkyPosition = pos; }
    void SetPacManDirection(glm::vec2 dir) { m_PacManDirection = dir; }

    void SetScatterTarget(glm::vec2 tile) { m_ScatterTarget = tile; }

    void SetChaseSprite(const std::string& path);
    void SetFrightenedSprite(const std::string& path);
    void SetEyesSprite(const std::string& path);
    void SetWhiteSprite(const std::string& path);
    void SetAnimator(dae::SpriteAnimatorComponent* animator) { m_pAnimator = animator; }

    void SetSprite(const std::string& path);
    void FlashSprite();

    void StartFlash() { if (m_FlashTimer <= 0.f) { m_FlashTimer = FLASH_INTERVAL; m_FlashToggle = false; } }
    void ResetFlash() { m_FlashTimer = 0.f; m_FlashToggle = false; m_IsFlashing = false; }
    void StopFlash();

    const std::string& GetChaseSprite()      const { return m_ChaseSprite; }
    const std::string& GetFrightenedSprite() const { return m_FrightenedSprite; }
    const std::string& GetEyesSprite()       const { return m_EyesSprite; }

    void  SetSpeed(float speed) { m_Speed = speed; }
	void SetIsFrightened(bool frightened) { m_IsFrightened = frightened; }
    float GetSpeed() const { return m_Speed; }
    void  SetPosition(glm::vec2 pos) { m_Position = pos; GetOwner().SetLocalPosition(pos.x, pos.y); }

    float GetNormalSpeed()     const { return m_NormalSpeed; }
    float GetFrightenedSpeed() const { return m_FrightenedSpeed; }
    float GetDeadSpeed()       const { return m_DeadSpeed; }

    void SetState(GhostState* newState);

    bool      IsPowerPelletActive() const { return m_PowerPelletActive; }
    bool      IsEatenByPacMan()     const { return m_EatenByPacMan; }
    bool      HasReachedBase()      const;

    glm::vec2 GetPacManPosition()   const { return m_PacManPosition; }
    glm::vec2 GetBasePosition()     const { return m_BasePosition; }
    glm::vec2 GetPosition()         const { return m_Position; }

   
    void SetPowerPelletActive(bool active);
    void SetEatenByPacMan(bool eaten) { m_EatenByPacMan = eaten; }
    void SetBasePosition(glm::vec2 pos) { m_BasePosition = pos; }

    void ChaseTarget(glm::vec2 worldTarget, float deltaTime);
    void FleeFrom(glm::vec2 worldTarget, float deltaTime);
    void Move(float deltaTime);
    void SetFrozen(bool frozen, float duration) { m_Frozen = frozen; m_floatFrozenTimer = duration; }
    void SetInitialFrozenTime(float t) { m_InitialFrozenTime = t; }
    void SetPaused(bool paused) { m_Paused = paused; }

    glm::vec2 GetTargetTile()        const;  
    glm::vec2 GetScatterTargetTile() const { return m_ScatterTarget; }
    void SetStartsOutside(bool startsOutside) { m_StartsOutside = startsOutside; }
    void SetHasExitedHouse(bool b) { m_HasExitedHouse = b; }


    void SetStartPosition(glm::vec2 pos) { m_StartPosition = pos; }
    void SetDoorWorldPosition(glm::vec2 pos) { m_DoorWorldPosition = pos; }
    void Reset();
    void Respawn();
    dae::GridComponent* GetGrid() const { return m_pGrid; }

    void SetPlayerControlled(bool controlled) { m_IsPlayerControlled = controlled; }
    bool IsPlayerControlled() const { return m_IsPlayerControlled; }
    void SetInputDirection(float x, float y) { m_InputDir = { x, y }; }
private:
    void MoveGridAI(glm::vec2 worldTarget, float deltaTime, bool flee);

    glm::vec2 TargetBlinky() const;
    glm::vec2 TargetPinky()  const;
    glm::vec2 TargetInky()   const;
    glm::vec2 TargetClyde()  const;

private:
    dae::GridComponent* m_pGrid{};
    dae::RenderComponent* m_pRender{};
    glm::vec2 m_StartPosition{};

    std::unique_ptr<GhostState> m_pCurrentState;

    Type m_Type{ Type::Blinky };

    glm::vec2 m_Position{};
    glm::vec2 m_Direction{ 1.f, 0.f };
    glm::ivec2 m_LastDecidedTile{ -1, -1 };
    glm::vec2  m_LastCommittedDirection{ 1.f, 0.f };

    glm::vec2 m_PacManPosition{};
    glm::vec2 m_PacManDirection{ 1.f, 0.f };
    glm::vec2 m_BlinkyPosition{};
    glm::vec2 m_BasePosition{};
    glm::vec2 m_ScatterTarget{};

    float m_Speed{ 97.f };
    float m_NormalSpeed{ 97.f };
    float m_FrightenedSpeed{ 60.f };
    float m_DeadSpeed{ 180.f };
    bool m_Frozen{ false };
    bool m_PowerPelletActive{ false };
    bool m_EatenByPacMan{ false };
    bool m_IsFrightened{ false };

    std::string m_ChaseSprite{};
    std::string m_FrightenedSprite{};
    std::string m_EyesSprite{};
    std::string m_WhiteSprite{};

    bool m_FlashToggle{ false };
    float m_FlashTimer{ 0.f };
    bool m_IsFlashing{ false };
    static constexpr float FLASH_INTERVAL{ 0.25f };
    static constexpr float REACH_BASE_RADIUS{ 8.f };

    static constexpr float CLYDE_CHASE_THRESHOLD{ 8.f };

    std::vector<glm::vec2> m_ExitPath;
    glm::vec2 m_DoorWorldPosition{};
    bool m_StartsOutside{ false };
    bool m_HasExitedHouse{ false };

	float m_floatFrozenTimer{ 0.f };
    float m_InitialFrozenTime{ 0.f };

    bool m_Paused{ false };

    bool m_IsPlayerControlled{ false };
    glm::vec2 m_InputDir{ 0.f, 0.f };

    dae::SpriteAnimatorComponent* m_pAnimator = nullptr;

};