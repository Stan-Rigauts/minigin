#include "GameObject.h"

namespace dae
{
    GameObject::~GameObject() = default;

    void GameObject::AddComponent(std::unique_ptr<Component> component)
    {
        component->SetOwner(this);
        m_Components.push_back(std::move(component));
    }

    void GameObject::RemoveComponent(Component* component)
    {
        auto it = std::remove_if(
            m_Components.begin(),
            m_Components.end(),
            [component](const std::unique_ptr<Component>& c) { return c.get() == component; });
        m_Components.erase(it, m_Components.end());
    }

    void GameObject::Update(float delta_sec)
    {
        for (auto& comp : m_Components)
            comp->Update(delta_sec);
    }

    void GameObject::FixedUpdate(float /*fixed_sec*/) {}
    void GameObject::Render() const {
        for (auto& comp : m_Components)
            comp->Render();
    }
}
