#include "DeadState.h"
#include "ChaseState.h"
#include "Ghost.h"

void DeadState::OnEnter()
{
    _ghost->SetSprite(_ghost->GetEyesSprite());
    _ghost->SetSpeed(_ghost->GetDeadSpeed());
}

GhostState* DeadState::HandleInput()
{
    return nullptr; 
}

void DeadState::Update(float deltaTime)
{
    _ghost->ChaseTarget(_ghost->GetBasePosition(), deltaTime);

    if (_ghost->HasReachedBase())
        _ghost->Respawn(); 
}