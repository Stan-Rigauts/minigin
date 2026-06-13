#pragma once
#include "GhostState.h"
#include "Ghost.h"
#include "ChaseState.h"
#include <glm/glm.hpp>
#include <vector>

class ExitHouseState : public GhostState
{
public:
    ExitHouseState(Ghost* ghost, std::vector<glm::vec2> waypoints)
        : GhostState(ghost), m_Waypoints(std::move(waypoints)), m_CurrentIndex(0)
    {
    }

    void OnEnter() override
    {
        _ghost->SetSprite(_ghost->GetChaseSprite());
        _ghost->SetSpeed(_ghost->GetNormalSpeed());

        if (!m_Waypoints.empty())
        {
            glm::vec2 pos = _ghost->GetPosition();
            float dist = glm::length(pos - m_Waypoints[0]);
            m_CurrentIndex = (dist < 8.f) ? 1 : 0;
        }
        else
            m_CurrentIndex = 0;

        if (_ghost->GetGrid())
            _ghost->GetGrid()->SetDoorLocked(false);
    }

    GhostState* HandleInput() override
    {
        if (m_Waypoints.empty() || !_ghost->GetGrid()) return nullptr;

        glm::vec2 target = m_Waypoints[m_CurrentIndex];
        glm::vec2 pos = _ghost->GetPosition();
        float dist = glm::length(pos - target);

        if (dist < m_ArrivalThreshold)
        {
            m_CurrentIndex++;

            if (m_CurrentIndex >= static_cast<int>(m_Waypoints.size()))
            {
                _ghost->GetGrid()->SetDoorLocked(true);
                return new ChaseState(_ghost);
            }
        }

        return nullptr;
    }

    void Update(float deltaTime) override
    {
        if (m_CurrentIndex >= static_cast<int>(m_Waypoints.size())) return;

        glm::vec2 target = m_Waypoints[m_CurrentIndex];
        glm::vec2 pos = _ghost->GetPosition();
        glm::vec2 diff = target - pos;
        float dist = glm::length(diff);

        if (dist < 0.001f) return; 

        glm::vec2 dir = diff / dist; 
        float     step = _ghost->GetNormalSpeed() * deltaTime;
        glm::vec2 newPos = pos + dir * std::min(step, dist); 

        _ghost->SetPosition(newPos);
    }

    void OnExit() override { _ghost->SetHasExitedHouse(true); }

    const char* GetName() const override { return "ExitHouse"; }

private:
    std::vector<glm::vec2> m_Waypoints;
    int   m_CurrentIndex = 0;
    float m_ArrivalThreshold = 4.0f; 
};