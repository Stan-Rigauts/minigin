#pragma once
#include "Component.h"

namespace dae
{
	class MoveComponent final : public Component
	{
	public:

		MoveComponent(GameObject& owner, float speed);

		void Move(float x, float y, float delta);

	private:

		float m_Speed{};
	};
}