#pragma once
namespace dae
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component() = default;

        virtual void FixedUpdate(float /*Fixed_sec*/) {}
        virtual void Update(float /*delta_sec*/) {}
        virtual void Render() const {} 

        void SetOwner(GameObject* owner) { m_Owner = owner; }
        GameObject* GetOwner() const { return m_Owner; }

    private:
        GameObject* m_Owner = nullptr;
    };

}