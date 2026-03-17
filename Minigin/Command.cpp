#include "Command.h"
#include "GameObject.h"
#include "MoveComponent.h"
#include "ScoreComponent.h"
#include "HealthComponent.h"

namespace dae
{
    void MoveCommand::Execute()
    {
        if (!m_Player) return;

        if (auto move = m_Player->GetComponent<MoveComponent>())
        {
            move->Move(m_DirX, m_DirY);
        }
    }

    void ScoreCommand::Execute()
    {
        if (m_pScore)
            m_pScore->AddScore(m_Amount);
    }

    void HealthCommand::Execute()
    {
        if (m_pHealth)
            m_pHealth->Damage(m_Damage);
    }
}