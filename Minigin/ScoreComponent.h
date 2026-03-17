#pragma once
#include "Component.h"
#include "Subject.h"

namespace dae
{
    class ScoreComponent final : public Component
    {
    public:
        ScoreComponent(GameObject& owner, int score = 0)
            : Component(owner)
            , m_Score(score)
        {
        }

        void AddScore(int amount);
        void ResetScore();

        int GetScore() const { return m_Score; }
        Subject& GetSubject() { return m_Subject; }

    private:
        int m_Score{};
        Subject m_Subject{};
    };
}