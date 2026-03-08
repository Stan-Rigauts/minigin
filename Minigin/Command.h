#pragma once

namespace dae
{
	class Command
	{
	public:
		virtual ~Command() = default;
		virtual void Execute() = 0;
	};

	class GameObject; 

	class MoveCommand : public Command
	{
	public:
		MoveCommand(GameObject* player, float x, float y);
		void Execute() override;

	private:
		GameObject* m_Player;
		float m_DirX;
		float m_DirY;
	};
}
