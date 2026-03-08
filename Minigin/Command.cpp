#include "Command.h"
#include "GameObject.h"
#include "MoveComponent.h"

MoveCommand::MoveCommand(GameObject* player, float x, float y, float delta)
    : m_Player(player), m_DirX(x), m_DirY(y), m_Delta(delta) {
}

void MoveCommand::Execute()
{
    if (!m_Player) return;
    auto move = m_Player->GetComponent<MoveComponent>();
    if (move)
        move->Move(m_DirX, m_DirY, m_Delta);
}