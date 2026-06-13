#include "ChaseState.h"
#include "FrightenedState.h"
#include "ScatterState.h"
#include "Ghost.h"

void ChaseState::OnEnter()
{
    
    _ghost->SetSprite(_ghost->GetChaseSprite());
    _ghost->SetSpeed(_ghost->GetNormalSpeed());
    _chaseTimer = 0.f;
}

GhostState* ChaseState::HandleInput()
{
    if (_ghost->IsPowerPelletActive())
        return new FrightenedState(_ghost);
    return nullptr;
}

void ChaseState::Update(float deltaTime)
{
    _chaseTimer += deltaTime;
    _ghost->ChaseTarget(_ghost->GetPacManPosition(),deltaTime);

    if (_chaseTimer >= CHASE_DURATION)
        _ghost->SetState(new ScatterState(_ghost));
}