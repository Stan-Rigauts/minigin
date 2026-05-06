#include "Ghost.h"
#include "GridComponent.h"
#include "RenderComponent.h"
#include "GameObject.h"
#include "ChaseState.h"

#include <glm/glm.hpp>
#include <array>
#include <limits>


Ghost::Ghost(dae::GameObject& owner)
    : dae::Component(owner)
{
}

void Ghost::Start()
{
    m_Position = { GetOwner().GetLocalPosition().x,
                                  GetOwner().GetLocalPosition().y };
    m_BasePosition = m_Position;
    m_Direction = { 1.f, 0.f };
    m_LastCommittedDirection = { 1.f, 0.f };
    m_LastDecidedTile = { -1, -1 }; 
 
    switch (m_Type)
    {
    case Type::Blinky: m_ScatterTarget = { 999.f,   0.f }; break;
    case Type::Pinky:  m_ScatterTarget = { 0.f,   0.f }; break; 
    case Type::Inky:   m_ScatterTarget = { 999.f, 999.f }; break;
    case Type::Clyde:  m_ScatterTarget = { 0.f, 999.f }; break; 
    }

    SetState(new ChaseState(this));
}

void Ghost::Update(float deltaTime)
{
    if (m_Frozen)
        return;

    if (!m_pRender)
        m_pRender = GetOwner().GetComponent<dae::RenderComponent>();

   
    if (m_FlashTimer > 0.f)
    {
        m_FlashTimer -= deltaTime;
        if (m_FlashTimer <= 0.f)
        {
            m_FlashToggle = !m_FlashToggle;
            m_FlashTimer = FLASH_INTERVAL;
            SetSprite(m_FlashToggle ? m_FrightenedSprite : m_EyesSprite);
        }
    }

    if (m_pCurrentState)
    {
        GhostState* next = m_pCurrentState->HandleInput();
        if (next)
        {
            m_pCurrentState->OnExit();
            m_pCurrentState.reset(next);
            m_pCurrentState->OnEnter();
        }

        m_pCurrentState->Update(deltaTime);
    }
}

void Ghost::SetState(GhostState* newState)
{
    if (m_pCurrentState)
        m_pCurrentState->OnExit();

    m_pCurrentState.reset(newState);

    if (m_pCurrentState)
        m_pCurrentState->OnEnter();
}


void Ghost::SetChaseSprite(const std::string& path)
{
    m_ChaseSprite = path;
    if (!m_pRender)
        m_pRender = GetOwner().GetComponent<dae::RenderComponent>();
    if (m_pRender)
    {
        m_pRender->SetTexture(path);
        m_pRender->SetSize(16.f, 16.f);
        m_pRender->SetCentered(true);
    }
}

void Ghost::SetFrightenedSprite(const std::string& path) { m_FrightenedSprite = path; }
void Ghost::SetEyesSprite(const std::string& path) { m_EyesSprite = path; }

void Ghost::SetSprite(const std::string& path)
{
    if (!m_pRender)
        m_pRender = GetOwner().GetComponent<dae::RenderComponent>();
    if (m_pRender)
    {
        m_pRender->SetTexture(path);
        m_pRender->SetSize(16.f, 16.f);
        m_pRender->SetCentered(true);
    }
}

void Ghost::FlashSprite()
{
    if (m_FlashTimer <= 0.f)
        m_FlashTimer = FLASH_INTERVAL;
}



bool Ghost::HasReachedBase() const
{
    return glm::length(m_Position - m_BasePosition) < REACH_BASE_RADIUS;
}


void Ghost::ChaseTarget(glm::vec2 worldTarget, float deltaTime)
{
    MoveGridAI(worldTarget, deltaTime, false);
}

void Ghost::FleeFrom(glm::vec2 worldTarget, float deltaTime)
{
    MoveGridAI(worldTarget, deltaTime, true);
}

void Ghost::Move(float deltaTime)
{
    MoveGridAI(GetTargetTile(), deltaTime, false);
}


glm::vec2 Ghost::TargetBlinky() const
{
    return m_PacManPosition;
}

glm::vec2 Ghost::TargetPinky() const
{
    if (!m_pGrid) return m_PacManPosition;
    const float ts = static_cast<float>(m_pGrid->GetTileSize());

    glm::vec2 dir = (m_PacManDirection == glm::vec2{ 0,0 })
        ? glm::vec2{ 1.f, 0.f }
    : m_PacManDirection;

    return m_PacManPosition + dir * (ts * 4.f);
}

glm::vec2 Ghost::TargetInky() const
{
    if (!m_pGrid) return m_PacManPosition;
    const float ts = static_cast<float>(m_pGrid->GetTileSize());

    glm::vec2 dir = (m_PacManDirection == glm::vec2{ 0,0 })
        ? glm::vec2{ 1.f, 0.f }
    : m_PacManDirection;

    glm::vec2 pivot = m_PacManPosition + dir * (ts * 2.f);

    glm::vec2 toBlinky = pivot - m_BlinkyPosition;
    return m_BlinkyPosition + toBlinky * 2.f;
}

glm::vec2 Ghost::TargetClyde() const
{
    if (!m_pGrid) return m_ScatterTarget;
    const float ts = static_cast<float>(m_pGrid->GetTileSize());
    const float threshold = CLYDE_CHASE_THRESHOLD * ts; 

    float dist = glm::length(m_PacManPosition - m_Position);
    return (dist > threshold) ? m_PacManPosition : m_ScatterTarget;
}


glm::vec2 Ghost::GetTargetTile() const
{
    switch (m_Type)
    {
    case Type::Blinky: return TargetBlinky();
    case Type::Pinky:  return TargetPinky();
    case Type::Inky:   return TargetInky();
    case Type::Clyde:  return TargetClyde();
    default:           return m_PacManPosition;
    }
}



void Ghost::Respawn()
{
    m_Position = m_StartPosition;
    m_Direction = { 1.f, 0.f };
    m_LastCommittedDirection = { 1.f, 0.f };
    m_LastDecidedTile = { -1, -1 };
    m_EatenByPacMan = false;
    m_PowerPelletActive = false;
    GetOwner().SetLocalPosition(m_Position.x, m_Position.y);
    SetState(new ChaseState(this));
}

void Ghost::MoveGridAI(glm::vec2 worldTarget, float deltaTime, bool flee)
{
    if (!m_pGrid) return;

    const int   ts = m_pGrid->GetTileSize();
    const float speed = m_Speed * deltaTime;
    const float snapDist = speed + 2.0f;

    int col = static_cast<int>((m_Position.x - m_pGrid->GetOffsetX()) / ts);
    int row = static_cast<int>((m_Position.y - m_pGrid->GetOffsetY()) / ts);

    glm::vec2 tileCenter{
        static_cast<float>(m_pGrid->GetOffsetX() + col * ts + ts / 2),
        static_cast<float>(m_pGrid->GetOffsetY() + row * ts + ts / 2)
    };

    float axialDist = (m_Direction.x != 0.f)
        ? std::abs(m_Position.x - tileCenter.x)
        : std::abs(m_Position.y - tileCenter.y);

    glm::ivec2 currentTile{ col, row };

    if (axialDist < snapDist && currentTile != m_LastDecidedTile)
    {
        if (m_Direction.x != 0.f)
            m_Position.y = tileCenter.y;
        else
            m_Position.x = tileCenter.x;

        constexpr std::array<glm::vec2, 4> DIRS{ {
            {  0.f, -1.f },
            {  0.f,  1.f },
            { -1.f,  0.f },
            {  1.f,  0.f }
        } };

        glm::vec2 reverse{ -m_LastCommittedDirection.x, -m_LastCommittedDirection.y };

        glm::vec2 bestDir = m_LastCommittedDirection;
        float     bestScore = flee
            ? -std::numeric_limits<float>::max()
            : std::numeric_limits<float>::max();

        for (const auto& dir : DIRS)
        {
            if (dir == reverse) continue;

            float nx = tileCenter.x + dir.x * ts;
            float ny = tileCenter.y + dir.y * ts;

            if (m_pGrid->IsWall(nx, ny)) continue;

          
            float d = glm::length(glm::vec2{ nx, ny } - worldTarget);

            bool better = flee ? (d > bestScore) : (d < bestScore);
            if (better)
            {
                bestScore = d;
                bestDir = dir;
            }
        }

        m_Direction = bestDir;
        m_LastCommittedDirection = bestDir;
        m_LastDecidedTile = currentTile;
    }

    m_Position += m_Direction * m_Speed * deltaTime;
    GetOwner().SetLocalPosition(m_Position.x, m_Position.y);
}