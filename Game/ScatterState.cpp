#include "ScatterState.h"
#include "FrightenedState.h"
#include "ChaseState.h"
#include "Ghost.h"

void ScatterState::OnEnter()
{
    _ghost->SetSprite(_ghost->GetChaseSprite());
    _ghost->SetSpeed(_ghost->GetNormalSpeed());
    _scatterTimer = 0.f;
}

GhostState* ScatterState::HandleInput()
{
    if (_ghost->IsPowerPelletActive())
        return new FrightenedState(_ghost);
    return nullptr;
}

void ScatterState::Update(float deltaTime)
{
    _scatterTimer += deltaTime;
    _ghost->ChaseTarget(_ghost->GetScatterTargetTile(), deltaTime); 

    if (_scatterTimer >= SCATTER_DURATION)
        _ghost->SetState(new ChaseState(_ghost));
}
