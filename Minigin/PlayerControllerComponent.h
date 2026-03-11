#pragma once
#include "Component.h"
#include "GameObject.h"
#include "MoveComponent.h"
#include "InputManager.h"

namespace dae
{
    class PlayerControllerComponent : public dae::Component
    {
    public:
        explicit PlayerControllerComponent(GameObject& owner, bool isKeyboard)
            : Component(owner),
            IsKeyboard{isKeyboard}
        {
             
        }

        void Update(float delta) override
        {
            auto& input = dae::InputManager::GetInstance();

            float x = 0.f, y = 0.f;
            if (IsKeyboard)
            {
                if (input.IsMoveLeft())  x -= 1.f;
                if (input.IsMoveRight()) x += 1.f;
                if (input.IsMoveUp())    y += 1.f;
                if (input.IsMoveDown())  y -= 1.f;

            }
            else
            {
                x = input.GetLeftStickX();
                y = input.GetLeftStickY();
            }
            
           
            if (x != 0.f || y != 0.f)
            {
                auto move = GetOwner().GetComponent<MoveComponent>();
                if (move)
                    move->Move(x, y, delta);
            }
        }
    private:
        bool IsKeyboard;
    };
}