#pragma once
#include "GhostState.h"

class ScatterState final : public GhostState
{
public:
    explicit ScatterState(Ghost* ghost) : GhostState(ghost) {}

    void OnEnter() override;
    GhostState* HandleInput() override;
    void Update(float deltaTime) override;
    const char* GetName() const override { return "Scatter"; }

private:
    float _scatterTimer{};
    static constexpr float SCATTER_DURATION{ 7.f };
};