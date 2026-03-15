#include "MoveComponent.h"
#include "GameObject.h"
#include "Transform.h"

namespace dae
{

	MoveComponent::MoveComponent(GameObject& owner, float speed)
		: Component(owner)
		, m_Speed(speed)
	{
	}

	void MoveComponent::Move(float x, float y)
	{
		auto& transform = GetOwner().GetTransform();
		auto pos = transform.GetWorldPosition();
		pos.x += x * m_Speed * m_Delta;
		pos.y -= y * m_Speed * m_Delta;
		transform.SetLocalPosition(pos);
	}

}