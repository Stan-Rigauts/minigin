#include "ScoreComponent.h"

namespace dae
{
    void ScoreComponent::AddScore(int amount)
    {
        m_Score += amount;
        m_Subject.Notify(GameEvent::ScoreChanged, m_Score);
    }

    void ScoreComponent::ResetScore()
    {
        m_Score = 0;
        m_Subject.Notify(GameEvent::ScoreReset, m_Score);
    }
}