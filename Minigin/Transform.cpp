#include "Transform.h"
#include "GameObject.h"
namespace dae
{
 
    void Transform::SetLocalPosition(float x, float y, float z)
    {
        m_LocalPosition = { x, y, z };
        SetPositionDirty();
    }

    void Transform::SetLocalPosition(const glm::vec3& pos)
    {
        m_LocalPosition = pos;
        SetPositionDirty();
    }

    const glm::vec3& Transform::GetWorldPosition()
    {
        if (m_PositionIsDirty)
            UpdateWorldPosition();
        return m_WorldPosition;
    }

    void Transform::UpdateWorldPosition()
    {
        if (m_pOwner->GetParent()  == nullptr)
            m_WorldPosition = m_LocalPosition;
        else
            m_WorldPosition = m_pOwner->GetParent()->GetWorldPosition() + m_LocalPosition;

        m_PositionIsDirty = false;
    }

    void Transform::SetPositionDirty()
    {
        if (m_PositionIsDirty) return;
        m_PositionIsDirty = true;

        if (!m_pOwner) return;
        if (m_pOwner->GetChildCount() == 0) return;

        for (auto* child : m_pOwner->GetChildren())
            child->GetTransform().SetPositionDirty();
    }


    void Transform::SetOwner(GameObject* owner)
    {
        m_pOwner = owner;
    }
}