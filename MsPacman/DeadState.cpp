#include "DeadState.h"
#include "ChaseState.h"
#include "Ghost.h"
#include "GridComponent.h" 
void DeadState::OnEnter()
{
    _ghost->SetSprite(_ghost->GetEyesSprite());
    _ghost->SetSpeed(_ghost->GetDeadSpeed());
    if (_ghost->GetGrid())
        _ghost->GetGrid()->SetDoorLocked(false);
}

GhostState* DeadState::HandleInput()
{
    return nullptr; 
}

void DeadState::Update(float deltaTime)
{
    if (!m_EnteredHouse)
    {
        _ghost->ChaseTarget(_ghost->GetBasePosition(), deltaTime);

        if (_ghost->HasReachedBase())
        {
            m_EnteredHouse = true;
            m_SinkTimer = 0.f;
        }
    }
    else
    {
        glm::vec2 pos = _ghost->GetPosition();
        pos.y += _ghost->GetFrightenedSpeed() * deltaTime;
        _ghost->SetPosition(pos);

        m_SinkTimer += deltaTime;
        if (m_SinkTimer >= SINK_DURATION)
            _ghost->Respawn();
    }
}