#include "GameObject.h"
#include <cassert>
#include <algorithm>

namespace dae
{
  
  
    GameObject::~GameObject()
    {
        if (m_pParent)
            m_pParent->RemoveChild(this);

        for (auto* child : m_pChildren)
            child->m_pParent = nullptr;
    }

  
    void GameObject::Update(float delta_sec)
    {
        for (auto& comp : m_Components)
            comp->Update(delta_sec);
    }

    void GameObject::FixedUpdate(float delta_sec)
    {
        for (auto& comp : m_Components)
            comp->FixedUpdate(delta_sec);
    }

    void GameObject::Render() const
    {
        for (auto& comp : m_Components)
            comp->Render();
    }

   
    void GameObject::AddComponent(std::unique_ptr<Component> component)
    {
        assert(component != nullptr);
        assert(&component->GetOwner() == this &&
            "Component owner does not match this GameObject!");
        m_Components.emplace_back(std::move(component));
    }

    void GameObject::RemoveComponent(Component* component)
    {
        auto it = std::remove_if(
            m_Components.begin(), m_Components.end(),
            [component](const std::unique_ptr<Component>& c) { return c.get() == component; });
        m_Components.erase(it, m_Components.end());
    }

   
    void GameObject::SetParent(GameObject* newParent, bool keepWorldPosition)
    {
        if (newParent == this || newParent == m_pParent || IsChild(newParent))
            return;

        if (newParent == nullptr)
        {
            m_Transform.SetLocalPosition(m_Transform.GetWorldPosition());
        }
        else
        {
            if (keepWorldPosition)
            {
                m_Transform.SetLocalPosition(
                    m_Transform.GetWorldPosition() - newParent->GetTransform().GetWorldPosition());
            }
            m_Transform.SetPositionDirty();
        }

        if (m_pParent)
            m_pParent->RemoveChild(this);

        m_pParent = newParent;

        if (m_pParent)
        {
            m_pParent->AddChild(this);
            m_Transform.SetParentTransform(&m_pParent->GetTransform());
        }
        else
        {
            m_Transform.SetParentTransform(nullptr);
        }
    }

    void GameObject::AddChild(GameObject* child)
    {
        assert(child != nullptr);
        assert(std::find(m_pChildren.begin(), m_pChildren.end(), child) == m_pChildren.end() &&
            "Child already present!");
        m_pChildren.push_back(child);
    }

    void GameObject::RemoveChild(GameObject* child)
    {
        auto it = std::find(m_pChildren.begin(), m_pChildren.end(), child);
        assert(it != m_pChildren.end() && "Child not found!");
        m_pChildren.erase(it);
    }

    bool GameObject::IsChild(const GameObject* candidate) const
    {
        for (const auto* child : m_pChildren)
        {
            if (child == candidate)        return true;
            if (child->IsChild(candidate)) return true;
        }
        return false;
    }
}