#pragma once

class Ghost;

class GhostState
{
public:
    explicit GhostState(Ghost* ghost) : _ghost(ghost) {}
    virtual ~GhostState() = default;

    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual GhostState* HandleInput() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual const char* GetName() const = 0;

protected:
    Ghost* _ghost;
};