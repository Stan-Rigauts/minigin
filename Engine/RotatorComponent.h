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
        RotatorComponent(GameObject& owner, float radius, float speed)
            : Component(owner), m_Radius{ radius }, m_Speed{ speed }, m_pOwner{owner}
        {
        }

        void Update(float delta_sec) override
        {
            if (m_Angle < (M_PI * 2))
            {
                m_Angle += m_Speed *
                    delta_sec;
            }
            else
            {
                m_Angle = 0.f;
            }

            glm::vec3 pivot = (!GetOwner().GetParent()) ? m_pOwner.GetParent()->GetTransform().GetWorldPosition() : glm::vec3{0, 0, 0};

            GetOwner().SetLocalPosition(
                pivot.x + std::cos(m_Angle) * m_Radius,
                pivot.y + std::sin(m_Angle) * m_Radius
            );
        }



        void SetRadius(float radius) { m_Radius = radius; }
        void SetSpeed(float speed) { m_Speed = speed; }

    private:
        float     m_Radius{};
        float     m_Speed{};
        float     m_Angle{};
        GameObject&  m_pOwner;
    };
}