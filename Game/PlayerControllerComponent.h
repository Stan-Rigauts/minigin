#pragma once
#include "Component.h"
#include "GameObject.h"
#include "MoveComponent.h"
#include "InputManager.h"
#include <cmath>

namespace dae
{
    class PlayerControllerComponent : public Component
    {
    public:
        explicit PlayerControllerComponent(GameObject& owner)
            : Component(owner)
        {
        }

        void Update(float) override
        {
            auto& input = InputManager::GetInstance();
            float x = input.GetLeftStickX();
            float y = input.GetLeftStickY();

            if (x == 0.f && y == 0.f)
                return;

            float length = std::sqrt(x * x + y * y);
            if (length > 1.f)
            {
                x /= length;
                y /= length;
            }

            auto move = GetOwner().GetComponent<MoveComponent>();
            if (move)
                move->Move(x, -y);  
        }
    };
}
