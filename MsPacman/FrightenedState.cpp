#include "FrightenedState.h"
#include "DeadState.h"
#include "ChaseState.h"
#include "Ghost.h"

void FrightenedState::OnEnter()
{
    _ghost->SetIsFrightened(true);
    _ghost->SetSprite(_ghost->GetFrightenedSprite());
    _ghost->SetSpeed(_ghost->GetFrightenedSpeed());
    _frightenTimer = 0.f; 
    _ghost->StopFlash();  
}
void FrightenedState::OnExit()
{
    _ghost->SetIsFrightened(false);
    _ghost->StopFlash();
}

GhostState* FrightenedState::HandleInput()
{
    if (_ghost->IsEatenByPacMan())
        return new DeadState(_ghost);
    return nullptr;
}

void FrightenedState::Update(float deltaTime)
{
    _frightenTimer += deltaTime;
    _ghost->FleeFrom(_ghost->GetPacManPosition(),deltaTime);

    if (_frightenTimer > FRIGHTEN_DURATION - FLASH_WARNING_TIME)
        _ghost->FlashSprite();

    if (_frightenTimer >= FRIGHTEN_DURATION)
        _ghost->SetState(new ChaseState(_ghost));
}