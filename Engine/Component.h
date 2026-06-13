#pragma once

namespace dae
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component(Component&&) = delete;
        Component& operator=(const Component&) = delete;
        Component& operator=(Component&&) = delete;

        virtual void FixedUpdate(float) {}
        virtual void Update(float) {}
        virtual void Render() const {}

        GameObject& GetOwner() const { return *m_pOwner; }
        void MarkForDestroy() { m_MarkedForDestroy = true; }
        bool IsMarkedForDestroy() const { return m_MarkedForDestroy; }

    
    protected:
        explicit Component(GameObject& owner)
            : m_pOwner(&owner)
        {
        }

    private:
        GameObject* const m_pOwner;  
        bool m_MarkedForDestroy{ false };
    };
}
