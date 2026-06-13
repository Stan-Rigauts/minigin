#include "MoveComponent.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "Transform.h"
#include <cmath>
namespace dae
{
    MoveComponent::MoveComponent(GameObject& owner, float speed)
        : Component(owner)
        , m_Speed(speed)
    {
    }

    void MoveComponent::Move(float dirX, float dirY)
    {
        m_QueuedDir = { dirX, dirY };
    }

    void MoveComponent::FreezeFor(float seconds)
    {
        m_Frozen = true;
        m_FreezeTimer = seconds;
    }

    void MoveComponent::Update(float delta)
    {

        if (m_FreezeTimer > 0.f)
        {
            m_FreezeTimer -= delta;
            if (m_FreezeTimer <= 0.f)
            {
                m_FreezeTimer = 0.f;
                m_Frozen = false;
            }
        }

        if (m_Frozen) return;
        m_Delta = delta;
        if (!m_Grid) return;

        if (IsAlignedToGrid() && m_QueuedDir != glm::vec2{ 0.f, 0.f })
        {
            if (CanMove(m_QueuedDir.x, m_QueuedDir.y))
            {
                m_CurrentDir = m_QueuedDir;
                m_QueuedDir = { 0.f, 0.f };
            }
        }

        if (m_CurrentDir != glm::vec2{ 0.f, 0.f })
        {
            if (IsAlignedToGrid() && !CanMove(m_CurrentDir.x, m_CurrentDir.y))
            {
                SnapToGrid();
                m_CurrentDir = { 0.f, 0.f };
            }
            else
            {
                auto& transform = GetOwner().GetTransform();
                auto pos = transform.GetWorldPosition();
                pos.x += m_CurrentDir.x * m_Speed * delta;
                pos.y += m_CurrentDir.y * m_Speed * delta;
                transform.SetLocalPosition(pos);
                ApplyTeleport();
            }
        }


        if (m_CurrentDir != glm::vec2{ 0.f, 0.f })
        {
            m_WakkaTimer -= delta;
            if (m_WakkaTimer <= 0.f)
            {
                servicelocator::get_sound_system().play(SND_RUNNING, 0.6f); 
                m_WakkaTimer = WAKKA_INTERVAL;
            }
        }
        else
        {
            m_WakkaTimer = 0.f;
        }

        UpdateAnimation();
    }

    void MoveComponent::UpdateAnimation()
    {
        if (!m_pAnimator) return;

        if (m_CurrentDir.x > 0.f) m_pAnimator->Play("walk_right");
        else if (m_CurrentDir.x < 0.f) m_pAnimator->Play("walk_left");
        else if (m_CurrentDir.y < 0.f) m_pAnimator->Play("walk_up");
        else if (m_CurrentDir.y > 0.f) m_pAnimator->Play("walk_down");
    }


    bool MoveComponent::IsAlignedToGrid() const
    {
        if (!m_Grid) return true;

        auto pos = GetOwner().GetTransform().GetWorldPosition();
        int tileSize = m_Grid->GetTileSize();
        int offsetX = m_Grid->GetOffsetX();
        int offsetY = m_Grid->GetOffsetY();

        float localX = pos.x - offsetX;
        float localY = pos.y - offsetY;

        float remX = std::fmod(localX, static_cast<float>(tileSize));
        float remY = std::fmod(localY, static_cast<float>(tileSize));

        const float threshold = 2.f;
        return (remX < threshold || remX > tileSize - threshold) &&
            (remY < threshold || remY > tileSize - threshold);
    }

    bool MoveComponent::CanMove(float dirX, float dirY) const
    {
        if (!m_Grid) return true;

        auto pos = GetOwner().GetTransform().GetWorldPosition();
        int tileSize = m_Grid->GetTileSize();
        float half = static_cast<float>(tileSize) / 2.f;

        float centerX = pos.x + half;
        float centerY = pos.y + half;

        float checkX = centerX + dirX * static_cast<float>(tileSize);
        float checkY = centerY + dirY * static_cast<float>(tileSize);

        const float mapLeft = static_cast<float>(m_Grid->GetOffsetX());
        const float mapRight = mapLeft + m_Grid->GetCols() * tileSize;

        if (dirX < 0.f && checkX < mapLeft)  return true;
        if (dirX > 0.f && checkX > mapRight) return true;

        return !m_Grid->IsWall(checkX, checkY);
    }


    void MoveComponent::SnapToGrid()
    {
        if (!m_Grid) return;

        auto& transform = GetOwner().GetTransform();
        auto pos = transform.GetWorldPosition();
        int tileSize = m_Grid->GetTileSize();
        int offsetX = m_Grid->GetOffsetX();
        int offsetY = m_Grid->GetOffsetY();

        float localX = pos.x - offsetX;
        float localY = pos.y - offsetY;

        float snappedX = std::round(localX / tileSize) * tileSize + offsetX;
        float snappedY = std::round(localY / tileSize) * tileSize + offsetY;

        transform.SetLocalPosition({ snappedX, snappedY, 0.f });
    }


    void MoveComponent::ApplyTeleport()
    {
        if (!m_Grid) return;

        auto& transform = GetOwner().GetTransform();
        auto pos = transform.GetWorldPosition();

        const float ts = static_cast<float>(m_Grid->GetTileSize());
        const float mapLeft = static_cast<float>(m_Grid->GetOffsetX());
        const float mapRight = mapLeft + m_Grid->GetCols() * ts;

        if (m_CurrentDir.x < 0.f && pos.x + ts <= mapLeft)
            pos.x = mapRight - ts;
        else if (m_CurrentDir.x > 0.f && pos.x >= mapRight)
            pos.x = mapLeft;

        transform.SetLocalPosition(pos);
    }
}