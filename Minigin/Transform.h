#pragma once
#include <glm/glm.hpp>
#include "Component.h"

namespace dae
{
	class Transform final : public Component
	{
	public:
		Transform(GameObject& owner);
		const glm::vec3& GetPosition() const { return m_position; }
		void SetPosition(float x, float y, float z = 0);
		void SetPosition(const glm::vec3& position);
	private:
		glm::vec3 m_position;
	};
}
