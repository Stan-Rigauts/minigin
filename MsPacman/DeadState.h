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
private:
    bool  m_EnteredHouse = false;
    float m_SinkTimer = 0.f;
    static constexpr float SINK_DURATION = 0.7f;
};