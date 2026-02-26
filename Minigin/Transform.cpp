#include "Transform.h"

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
        if (m_pParent == nullptr)
            m_WorldPosition = m_LocalPosition;
        else
            m_WorldPosition = m_pParent->GetWorldPosition() + m_LocalPosition;

        m_PositionIsDirty = false;
    }

    void Transform::SetPositionDirty()
    {
        m_PositionIsDirty = true;
    }
}