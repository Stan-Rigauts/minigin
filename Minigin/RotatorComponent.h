#pragma once
#include "Component.h"
#include "GameObject.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace dae
{
    class RotatorComponent final : public Component
    {
    public:
        RotatorComponent(GameObject& owner, float radius, float speed, const glm::vec3& center)
            : Component(owner), m_Radius{ radius }, m_Speed{ speed }, m_Center{ center } {
        }

        RotatorComponent(GameObject& owner, float radius, float speed)
            : Component(owner), m_Radius{ radius }, m_Speed{ speed }
            , m_UseParent{ true } {
        }

        void Update(float delta_sec) override
        {
            m_Angle += m_Speed * delta_sec;

            glm::vec3 pivot = (!m_UseParent || !GetOwner().GetParent())
                ? m_Center
                : glm::vec3{ 0, 0, 0 };   

            GetOwner().SetLocalPosition(
                pivot.x + std::cos(m_Angle) * m_Radius,
                pivot.y + std::sin(m_Angle) * m_Radius
            );
        }

        void Render() const override {}

        void SetRadius(float radius) { m_Radius = radius; }
        void SetSpeed(float speed) { m_Speed = speed; }
        void SetCenter(const glm::vec3& center) { m_Center = center; m_UseParent = false; }

    private:
        float     m_Radius{};
        float     m_Speed{};
        float     m_Angle{};
        glm::vec3 m_Center{};
        bool      m_UseParent{ false };
    };
}