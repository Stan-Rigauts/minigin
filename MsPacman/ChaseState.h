#pragma once
#include "GhostState.h"

class ChaseState final : public GhostState
{
public:
    explicit ChaseState(Ghost* ghost) : GhostState(ghost) {}

    void OnEnter() override;
    GhostState* HandleInput() override;
    void Update(float deltaTime) override;
    const char* GetName() const override { return "Chase"; }

private:
    float _chaseTimer{};
    static constexpr float CHASE_DURATION{ 20.f };
};