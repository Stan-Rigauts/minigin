#include "Ghost.h"
#include "GridComponent.h"
#include "RenderComponent.h"
#include "GameObject.h"
#include "ChaseState.h"
#include "ExitHouseState.h"
#include "FrightenedState.h"
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
    m_Direction = { -1.f, 0.f };
    m_LastCommittedDirection = { 1.f, 0.f };
    m_LastDecidedTile = { -1, -1 };

    switch (m_Type)
    {
    case Type::Blinky: m_ScatterTarget = { 999.f,   0.f }; break;
    case Type::Pinky:  m_ScatterTarget = { 0.f,   0.f }; break;
    case Type::Inky:   m_ScatterTarget = { 999.f, 999.f }; break;
    case Type::Clyde:  m_ScatterTarget = { 0.f, 999.f }; break;
    }

    if (m_StartsOutside)
    {
        m_HasExitedHouse = true;
        if (m_pGrid)
            m_pGrid->SetDoorLocked(true);
        SetState(new ChaseState(this));
    }
    else
        SetState(new ExitHouseState(this, m_ExitPath));
}

void Ghost::Update(float deltaTime)
{
    if (m_Paused) return;

    m_floatFrozenTimer -= deltaTime;
    if (m_floatFrozenTimer <= 0.f)
        m_Frozen = false;

    if (m_Frozen) return;

    if (!m_pRender)
        m_pRender = GetOwner().GetComponent<dae::RenderComponent>();

    if (m_FlashTimer > 0.f)
    {
        m_FlashTimer -= deltaTime;
        if (m_FlashTimer <= 0.f)
        {
            m_FlashToggle = !m_FlashToggle;
            m_FlashTimer = FLASH_INTERVAL;
            if (m_IsFrightened)
                SetSprite(m_FlashToggle ? m_FrightenedSprite : m_WhiteSprite);
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

        if (m_IsPlayerControlled)
        {
            UpdatePlayerControlled(deltaTime);
            UpdateAnimation();
            m_InputDir = { 0.f, 0.f };
            return;
        }

        m_pCurrentState->Update(deltaTime);
    }

    UpdateAnimation();
}

void Ghost::UpdateAnimation()
{
    if (!m_pAnimator) return;

    if (m_EatenByPacMan)
    {
        if (m_Direction.x > 0.f) m_pAnimator->Play("eyes_right");
        else if (m_Direction.x < 0.f) m_pAnimator->Play("eyes_left");
        else if (m_Direction.y < 0.f) m_pAnimator->Play("eyes_up");
        else                           m_pAnimator->Play("eyes_down");
        return;
    }

    if (m_PowerPelletActive)
    {
        m_pAnimator->SetBaseTexture(m_FrightenedSprite);
        if (m_FlashTimer > 0.f)
            m_pAnimator->Play(m_FlashToggle ? "white" : "frightened");
        else
            m_pAnimator->Play("frightened");
        return;
    }

    m_pAnimator->SetBaseTexture(m_ChaseSprite);

    if (m_Direction.x > 0.f) m_pAnimator->Play("walk_right");
    else if (m_Direction.x < 0.f) m_pAnimator->Play("walk_left");
    else if (m_Direction.y < 0.f) m_pAnimator->Play("walk_up");
    else                           m_pAnimator->Play("walk_down");
}

void Ghost::UpdatePlayerControlled(float deltaTime)
{
    if (!m_pGrid) return;

    if (m_EatenByPacMan)
    {
        m_pCurrentState->Update(deltaTime);
        return;
    }

    float currentSpeed = m_PowerPelletActive ? m_FrightenedSpeed : m_NormalSpeed;
    const float speed = currentSpeed * deltaTime;

    const int tileSize = m_pGrid->GetTileSize();

    int col = static_cast<int>((m_Position.x - m_pGrid->GetOffsetX()) / tileSize);
    int row = static_cast<int>((m_Position.y - m_pGrid->GetOffsetY()) / tileSize);

    glm::vec2 tileCenter{
        static_cast<float>(m_pGrid->GetOffsetX() + col * tileSize + tileSize / 2),
        static_cast<float>(m_pGrid->GetOffsetY() + row * tileSize + tileSize / 2)
    };

    float perpDist = (m_Direction.x != 0.f)
        ? std::abs(m_Position.y - tileCenter.y)
        : std::abs(m_Position.x - tileCenter.x);

    if (m_Direction.x != 0.f)
        m_Position.y = tileCenter.y;
    else if (m_Direction.y != 0.f)
        m_Position.x = tileCenter.x;

    if (perpDist <= 1.0f && glm::length(m_InputDir) > 0.f)
    {
        glm::vec2 reverse{ -m_Direction.x, -m_Direction.y };
        if (m_InputDir != reverse)
        {
            float turnX = tileCenter.x + m_InputDir.x * tileSize;
            float turnY = tileCenter.y + m_InputDir.y * tileSize;
            if (!m_pGrid->IsWall(turnX, turnY))
                m_Direction = m_InputDir;
        }
    }

    const float mapLeft = static_cast<float>(m_pGrid->GetOffsetX());
    const float mapRight = mapLeft + m_pGrid->GetCols() * tileSize;

    bool isTunnel = (m_Direction.x < 0.f && tileCenter.x - tileSize < mapLeft)
        || (m_Direction.x > 0.f && tileCenter.x + tileSize > mapRight);

    float wallCheckX = tileCenter.x + m_Direction.x * tileSize;
    float wallCheckY = tileCenter.y + m_Direction.y * tileSize;
    bool isDoor = m_pGrid->GetTileAtWorld(wallCheckX, wallCheckY) == dae::TileType::GhostDoor;
    bool wallAhead = !isTunnel && !isDoor && m_pGrid->IsWall(wallCheckX, wallCheckY);

    glm::vec2 move = m_Direction * speed;

    if (wallAhead)
    {
        if (m_Direction.x > 0.f)
        {
            m_Position.x = std::min(m_Position.x + move.x, tileCenter.x);
            m_Position.y = tileCenter.y;
        }
        else if (m_Direction.x < 0.f)
        {
            m_Position.x = std::max(m_Position.x + move.x, tileCenter.x);
            m_Position.y = tileCenter.y;
        }
        else if (m_Direction.y > 0.f)
        {
            m_Position.y = std::min(m_Position.y + move.y, tileCenter.y);
            m_Position.x = tileCenter.x;
        }
        else if (m_Direction.y < 0.f)
        {
            m_Position.y = std::max(m_Position.y + move.y, tileCenter.y);
            m_Position.x = tileCenter.x;
        }
    }
    else
    {
        m_Position += move;
        if (m_Direction.x != 0.f)
            m_Position.y = tileCenter.y;
        else if (m_Direction.y != 0.f)
            m_Position.x = tileCenter.x;
    }

    if (m_Direction.x < 0.f && m_Position.x + tileSize <= mapLeft)
        m_Position.x = mapRight - tileSize;
    else if (m_Direction.x > 0.f && m_Position.x >= mapRight)
        m_Position.x = mapLeft;

    GetOwner().SetLocalPosition(m_Position.x, m_Position.y);
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
        m_pRender->SetSourceRect(0, 0, 16, 16);
        m_pRender->SetSize(28.f, 28.f);
        m_pRender->SetCentered(true);
    }
}

void Ghost::SetFrightenedSprite(const std::string& path) { m_FrightenedSprite = path; }
void Ghost::SetEyesSprite(const std::string& path) { m_EyesSprite = path; }
void Ghost::SetWhiteSprite(const std::string& path) { m_WhiteSprite = path; }

void Ghost::SetSprite(const std::string& path)
{
    if (!m_pRender)
        m_pRender = GetOwner().GetComponent<dae::RenderComponent>();
    if (m_pRender)
    {
        m_pRender->SetTexture(path);
        m_pRender->SetSourceRect(0, 0, 16, 16);
        m_pRender->SetSize(28.f, 28.f);
        m_pRender->SetCentered(true);
    }
}

void Ghost::FlashSprite()
{
    if (m_FlashTimer > 0.f) return;
    m_FlashTimer = FLASH_INTERVAL;
    m_FlashToggle = false;
}

void Ghost::StopFlash()
{
    m_FlashTimer = 0.f;
    m_FlashToggle = false;
    SetSprite(m_ChaseSprite);
}

bool Ghost::HasReachedBase() const
{
    return glm::length(m_Position - m_BasePosition) < REACH_BASE_RADIUS;
}

void Ghost::SetPowerPelletActive(bool active)
{
    if (active && !m_HasExitedHouse) return;

    bool wasActive = m_PowerPelletActive;
    m_PowerPelletActive = active;
    m_IsFrightened = active;
    m_FlashTimer = 0.f;
    m_FlashToggle = false;

    if (active)
    {
        m_Speed = m_FrightenedSpeed;
        if (wasActive)
            SetState(new FrightenedState(this));  
        SetSprite(m_FrightenedSprite);
    }
    else
    {
        m_Speed = m_NormalSpeed;
    }
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

    glm::vec2 dir = (m_PacManDirection == glm::vec2{ 0, 0 })
        ? glm::vec2{ 1.f, 0.f }
    : m_PacManDirection;

    return m_PacManPosition + dir * (ts * 4.f);
}

glm::vec2 Ghost::TargetInky() const
{
    if (!m_pGrid) return m_PacManPosition;
    const float ts = static_cast<float>(m_pGrid->GetTileSize());

    glm::vec2 dir = (m_PacManDirection == glm::vec2{ 0, 0 })
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

void Ghost::Reset()
{
    m_Position = m_StartPosition;
    m_Direction = { 1.f, 0.f };
    m_LastCommittedDirection = { 1.f, 0.f };
    m_LastDecidedTile = { -1, -1 };
    m_EatenByPacMan = false;
    m_HasExitedHouse = false;
    m_IsFrightened = false;
    m_PowerPelletActive = false;
    GetOwner().SetLocalPosition(m_Position.x, m_Position.y);
    SetFrozen(true, m_InitialFrozenTime - 4);

    if (m_StartsOutside)
    {
        m_HasExitedHouse = true;
        SetState(new ChaseState(this));
    }
    else
    {
        SetState(new ExitHouseState(this, m_ExitPath));
    }
}


void Ghost::Respawn()
{
    GetGrid()->SetDoorLocked(false);
    m_Direction = { 0.f, -1.f };
    m_LastCommittedDirection = { 0.f, -1.f };
    m_LastDecidedTile = { -1, -1 };
    m_EatenByPacMan = false;
    m_HasExitedHouse = false;
    m_PowerPelletActive = false;
    GetOwner().SetLocalPosition(m_Position.x, m_Position.y);
    SetFrozen(true, 2.0f);

    std::vector<glm::vec2> respawnPath = { m_Position };
    for (const auto& wp : m_ExitPath)
        respawnPath.push_back(wp);

    SetState(new ExitHouseState(this, respawnPath));
}

void Ghost::MoveGridAI(glm::vec2 worldTarget, float deltaTime, bool flee)
{
    if (!m_pGrid) return;
    const int   tileSize = m_pGrid->GetTileSize();
    const float speed = m_Speed * deltaTime;
    const float snapDist = speed + 2.0f;

    int col = static_cast<int>((m_Position.x - m_pGrid->GetOffsetX()) / tileSize);
    int row = static_cast<int>((m_Position.y - m_pGrid->GetOffsetY()) / tileSize);

    glm::vec2 tileCenter{
        static_cast<float>(m_pGrid->GetOffsetX() + col * tileSize + tileSize / 2),
        static_cast<float>(m_pGrid->GetOffsetY() + row * tileSize + tileSize / 2)
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
        glm::vec2 bestDir = reverse;
        float     bestScore = flee ? -std::numeric_limits<float>::max()
            : std::numeric_limits<float>::max();
        bool      foundForward = false;

        const float mapLeft = static_cast<float>(m_pGrid->GetOffsetX());
        const float mapRight = mapLeft + m_pGrid->GetCols() * tileSize;

        for (const auto& direction : DIRS)
        {
            if (direction == reverse) continue;

            float nx = tileCenter.x + direction.x * tileSize;
            float ny = tileCenter.y + direction.y * tileSize;

            bool isTunnel = (direction.x < 0.f && nx < mapLeft)
                || (direction.x > 0.f && nx > mapRight);

            if (!isTunnel && m_pGrid->IsWall(nx, ny)) continue;

            if (!isTunnel && m_HasExitedHouse && !m_EatenByPacMan &&
                m_pGrid->GetTileAtWorld(nx, ny) == dae::TileType::GhostDoor)
                continue;

            float distance = glm::length(glm::vec2{ nx, ny } - worldTarget);
            bool  better = flee ? (distance > bestScore) : (distance < bestScore);
            if (better)
            {
                bestScore = distance;
                bestDir = direction;
                foundForward = true;
            }
        }

        if (!foundForward)
        {
            float nx = tileCenter.x + reverse.x * tileSize;
            float ny = tileCenter.y + reverse.y * tileSize;
            if (m_pGrid->IsWall(nx, ny))
                bestDir = m_LastCommittedDirection;
        }

        m_Direction = bestDir;
        m_LastCommittedDirection = bestDir;
        m_LastDecidedTile = currentTile;
    }

    m_Position += m_Direction * m_Speed * deltaTime;

    const float mapLeft = static_cast<float>(m_pGrid->GetOffsetX());
    const float mapRight = mapLeft + m_pGrid->GetCols() * tileSize;

    if (m_Direction.x < 0.f && m_Position.x + tileSize <= mapLeft)
        m_Position.x = mapRight - tileSize;
    else if (m_Direction.x > 0.f && m_Position.x >= mapRight)
        m_Position.x = mapLeft;

    GetOwner().SetLocalPosition(m_Position.x, m_Position.y);
}