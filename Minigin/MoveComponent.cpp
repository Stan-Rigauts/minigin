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

	void MoveComponent::Move(float x, float y, float delta)
	{
		auto& transform = GetOwner().GetTransform();

		auto pos = transform.GetWorldPosition();

		pos.x += x * m_Speed * delta;
		pos.y -= y * m_Speed * delta;

		transform.SetLocalPosition(pos);
	}

}