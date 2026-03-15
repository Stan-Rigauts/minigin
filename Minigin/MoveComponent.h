#pragma once
#include "Component.h"

namespace dae
{
	class MoveComponent final : public Component
	{
	public:

		MoveComponent(GameObject& owner, float speed);

		void Update(float delta) override { m_Delta = delta; }
		void Move(float x, float y); 
	

	private:

		float m_Delta{};
		float m_Speed{};
	};
}