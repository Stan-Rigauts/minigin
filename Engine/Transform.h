#pragma once
#include <glm/glm.hpp>
#include <memory>
namespace dae
{
    class GameObject;
    class Transform final
    {
    public:
        Transform()=default;

        void             SetLocalPosition(float x, float y, float z = 0.f);
        void             SetLocalPosition(const glm::vec3& pos);
        const glm::vec3& GetLocalPosition() const { return m_LocalPosition; }

        const glm::vec3& GetWorldPosition();

        bool IsDirty() const { return m_PositionIsDirty; }

        void SetPositionDirty();
        void SetOwner(GameObject* owner);

    private:
        void UpdateWorldPosition();

        glm::vec3  m_LocalPosition{};
        glm::vec3  m_WorldPosition{};
        bool       m_PositionIsDirty{ true };

        GameObject* m_pOwner{nullptr};
    };
}