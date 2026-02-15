#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "Component.h"

namespace dae
{
    class GameObject
    {
    public:
        GameObject() = default;
        virtual ~GameObject();

        GameObject(const GameObject& other) = delete;
        GameObject(GameObject&& other) = delete;
        GameObject& operator=(const GameObject& other) = delete;
        GameObject& operator=(GameObject&& other) = delete;

        virtual void Update(float delta_sec);
        virtual void FixedUpdate(float fixed_sec);
        virtual void Render() const;

        void AddComponent(std::unique_ptr<Component> component);
        void RemoveComponent(Component* component);

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
    };
}
