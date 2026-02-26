#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "glm/glm.hpp"
#include "Component.h"
#include "Transform.h"

namespace dae
{
    class GameObject final
    {
    public:
        GameObject() = default;
        ~GameObject();

        GameObject(const GameObject& other) = delete;
        GameObject(GameObject&& other) = delete;
        GameObject& operator=(const GameObject& other) = delete;
        GameObject& operator=(GameObject&& other) = delete;

       
        void Update(float delta_sec);
        void FixedUpdate(float delta_sec);
        void Render() const;

      
        void AddComponent(std::unique_ptr<Component> component);
        void RemoveComponent(Component* component);

        template<typename T>
        T* GetComponent() const
        {
            for (const auto& comp : m_Components)
            {
                if (auto* cast = dynamic_cast<T*>(comp.get()))
                    return cast;
            }
            return nullptr;
        }

        template<typename T>
        bool HasComponent() const { return GetComponent<T>() != nullptr; }

       
        Transform& GetTransform() { return m_Transform; }
        const Transform& GetTransform() const { return m_Transform; }

        void             SetLocalPosition(float x, float y, float z = 0.f) { m_Transform.SetLocalPosition(x, y, z); }
        void             SetLocalPosition(const glm::vec3& pos) { m_Transform.SetLocalPosition(pos); }
        const glm::vec3& GetLocalPosition()  const { return m_Transform.GetLocalPosition(); }
        const glm::vec3& GetWorldPosition() { return m_Transform.GetWorldPosition(); }

       
        void MarkForDestroy() { m_IsMarkedForDestroy = true; }
        bool IsMarkedForDestroy() const { return m_IsMarkedForDestroy; }

    
        void        SetParent(GameObject* parent, bool keepWorldPosition = true);
        GameObject* GetParent()              const { return m_pParent; }
        size_t      GetChildCount()          const { return m_pChildren.size(); }
        GameObject* GetChildAt(size_t index) const { return m_pChildren[index]; }

        
        std::vector<GameObject*>::iterator       begin() { return m_pChildren.begin(); }
        std::vector<GameObject*>::iterator       end() { return m_pChildren.end(); }
        std::vector<GameObject*>::const_iterator begin() const { return m_pChildren.begin(); }
        std::vector<GameObject*>::const_iterator end()   const { return m_pChildren.end(); }

    private:
    
        void AddChild(GameObject* child);
        void RemoveChild(GameObject* child);
        bool IsChild(const GameObject* candidate) const;

       
        Transform   m_Transform{};
        bool        m_IsMarkedForDestroy{ false };

        GameObject* m_pParent{ nullptr };
        std::vector<GameObject*> m_pChildren{};

        std::vector<std::unique_ptr<Component>> m_Components{};
    };
}