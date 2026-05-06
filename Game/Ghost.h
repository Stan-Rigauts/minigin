#pragma once
#include "Component.h"
#include "GhostState.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace dae
{
    class GameObject;
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
    void Start();

    void SetGrid(dae::GridComponent* grid) { m_pGrid = grid; }
    void SetType(Type type) { m_Type = type; }

    void SetPacManPosition(glm::vec2 pos) { m_PacManPosition = pos; }
    void SetBlinkyPosition(glm::vec2 pos) { m_BlinkyPosition = pos; }
    void SetPacManDirection(glm::vec2 dir) { m_PacManDirection = dir; }

    void SetScatterTarget(glm::vec2 tile) { m_ScatterTarget = tile; }

    void SetChaseSprite(const std::string& path);
    void SetFrightenedSprite(const std::string& path);
    void SetEyesSprite(const std::string& path);

    void SetSprite(const std::string& path);
    void FlashSprite();

    const std::string& GetChaseSprite()      const { return m_ChaseSprite; }
    const std::string& GetFrightenedSprite() const { return m_FrightenedSprite; }
    const std::string& GetEyesSprite()       const { return m_EyesSprite; }

    void  SetSpeed(float s) { m_Speed = s; }
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

   
    void SetPowerPelletActive(bool b) { m_PowerPelletActive = b; }
    void SetEatenByPacMan(bool b) { m_EatenByPacMan = b; }
    void SetBasePosition(glm::vec2 pos) { m_BasePosition = pos; }

    void ChaseTarget(glm::vec2 worldTarget, float deltaTime);
    void FleeFrom(glm::vec2 worldTarget, float deltaTime);
    void Move(float deltaTime);
    void SetFrozen(bool f) { m_Frozen = f; }
    glm::vec2 GetTargetTile()        const;  
    glm::vec2 GetScatterTargetTile() const { return m_ScatterTarget; }

    void SetStartPosition(glm::vec2 pos) { m_StartPosition = pos; }
    void Respawn();

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

    float m_Speed{ 100.f };
    float m_NormalSpeed{ 100.f };
    float m_FrightenedSpeed{ 60.f };
    float m_DeadSpeed{ 180.f };
    bool m_Frozen{ false };
    bool m_PowerPelletActive{ false };
    bool m_EatenByPacMan{ false };

    std::string m_ChaseSprite{};
    std::string m_FrightenedSprite{};
    std::string m_EyesSprite{};

    bool m_FlashToggle{ false };
    float m_FlashTimer{ 0.f };
    static constexpr float FLASH_INTERVAL{ 0.25f };
    static constexpr float REACH_BASE_RADIUS{ 8.f };

    static constexpr float CLYDE_CHASE_THRESHOLD{ 8.f };

   

    
};