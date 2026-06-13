#pragma once
#include "GhostState.h"

class FrightenedState final : public GhostState
{
public:
    explicit FrightenedState(Ghost* ghost) : GhostState(ghost) {}

    void OnEnter() override;
    void OnExit()override;
    GhostState* HandleInput() override;
    void Update(float deltaTime) override;
    const char* GetName() const override { return "Frightened"; }

private:
    float _frightenTimer{};
    static constexpr float FRIGHTEN_DURATION{ 8.f };
    static constexpr float FLASH_WARNING_TIME{ 2.f };
};