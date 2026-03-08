#pragma once
#include "Component.h"
#include "GameObject.h"
#include "MoveComponent.h"
#include "InputManager.h"
#include <iostream>
#include <windows.h>
namespace dae
{
    class PlayerControllerComponent : public dae::Component
    {
    public:
        explicit PlayerControllerComponent(GameObject& owner) : Component(owner) {}

        void Update(float delta) override
        {
            auto& input = dae::InputManager::GetInstance();
            float x = input.GetLeftStickX();
            float y = input.GetLeftStickY();


            // inside Update():
            XINPUT_STATE state{};
            DWORD result = XInputGetState(0, &state);
            OutputDebugStringA(("Result: " + std::to_string(result) +
                "  Raw X: " + std::to_string(state.Gamepad.sThumbLX) + "\n").c_str());


            if (x != 0.f || y != 0.f)
            {
                auto move = GetOwner().GetComponent<MoveComponent>();
                if (move)
                    move->Move(x, y, delta);
            }
        }
    };

}