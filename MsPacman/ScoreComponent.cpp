#include "ScoreComponent.h"
#include "GameEvent.h" 
namespace dae
{
    void ScoreComponent::AddScore(int amount)
    {
        m_Score += amount;
        m_Subject.Notify(static_cast<int>(GameEvent::ScoreChanged), &GetOwner());
    }
    void ScoreComponent::ResetScore()
    {
        m_Score = 0;
        m_Subject.Notify(static_cast<int>(GameEvent::ScoreReset), &GetOwner());
    }
}