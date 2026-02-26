#pragma once
#include <glm/glm.hpp>

namespace dae
{
    class Transform final
    {
    public:
        Transform() = default;

        void             SetLocalPosition(float x, float y, float z = 0.f);
        void             SetLocalPosition(const glm::vec3& pos);
        const glm::vec3& GetLocalPosition() const { return m_LocalPosition; }

        const glm::vec3& GetWorldPosition();

        void SetParentTransform(Transform* parent) { m_pParent = parent; SetPositionDirty(); }

        void SetPositionDirty();

    private:
        void UpdateWorldPosition();

        glm::vec3  m_LocalPosition{};
        glm::vec3  m_WorldPosition{};
        bool       m_PositionIsDirty{ true };

        Transform* m_pParent{ nullptr };   
    };
}