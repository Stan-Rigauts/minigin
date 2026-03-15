#include "Command.h"
#include "GameObject.h"
#include "MoveComponent.h"

dae::MoveCommand::MoveCommand(GameObject* player, float x, float y)
    : m_Player(player), m_DirX(x), m_DirY(y) {
}

void dae::MoveCommand::Execute()
{
    if (!m_Player) return;
    auto move = m_Player->GetComponent<MoveComponent>();
    if (move)
        move->Move(m_DirX, m_DirY);  
}