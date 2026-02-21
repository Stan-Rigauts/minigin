#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "Component.h"

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
        void FixedUpdate(float fixed_sec);
        void Render() const;

        void AddComponent(std::unique_ptr<Component> component);
        void RemoveComponent(Component* component);

        void MarkForDestroy() { m_IsMarkedForDestroy = true; }
        bool IsMarkedForDestroy() const { return m_IsMarkedForDestroy; }

         template<typename T>
        T* GetComponent()
        {
            for (auto& c : m_Components)
                if (auto ptr = dynamic_cast<T*>(c.get()))
                    return ptr;
            return nullptr;
        }

        template<typename T>
        bool HasComponent()
        {
            return GetComponent<T>() != nullptr;
        }

    private:
        std::vector<std::unique_ptr<Component>> m_Components;

        bool m_IsMarkedForDestroy{ false }; 
    };
}
