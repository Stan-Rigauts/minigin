#pragma once
#include "GhostState.h"

class DeadState final : public GhostState
{
public:
    explicit DeadState(Ghost* ghost) : GhostState(ghost) {}

    void OnEnter() override;
    GhostState* HandleInput() override;
    void Update(float deltaTime) override;
    const char* GetName() const override { return "Dead"; }

};