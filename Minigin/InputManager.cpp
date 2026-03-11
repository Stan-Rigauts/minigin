#include "InputManager.h"
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <cmath>

namespace dae
{
    bool InputManager::ProcessInput()
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                return false;

            if (e.type == SDL_EVENT_KEY_DOWN)
            {
                switch (e.key.scancode)
                {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:    m_MoveUp = true; break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:  m_MoveDown = true; break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:  m_MoveLeft = true; break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT: m_MoveRight = true; break;
                default: break;
                }
            }
            if (e.type == SDL_EVENT_KEY_UP)
            {
                switch (e.key.scancode)
                {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:    m_MoveUp = false; break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:  m_MoveDown = false; break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:  m_MoveLeft = false; break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT: m_MoveRight = false; break;
                default: break;
                }
            }

            ImGui_ImplSDL3_ProcessEvent(&e);
        }


        //controller

        m_PreviousState = m_CurrentState;
        m_CurrentState = {};

#ifdef __EMSCRIPTEN__

        // SDL3

        if (!m_SDLController)
        {
            int count = 0;
            SDL_JoystickID* joysticks = SDL_GetJoysticks(&count);

            if (count > 0)
            {
                if (SDL_IsGamepad(joysticks[0]))
                    m_SDLController = SDL_OpenGamepad(joysticks[0]);
            }

            SDL_free(joysticks);
        }



        if (m_SDLController)
        {
            Uint16 buttons = 0;

            for (int b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; ++b)
            {
                if (SDL_GetGamepadButton(m_SDLController, (SDL_GamepadButton)b))
                    buttons |= (1 << b);
            }

            m_CurrentState.Gamepad.wButtons = buttons;

            m_CurrentState.Gamepad.sThumbLX =
                (short)SDL_GetGamepadAxis(m_SDLController, SDL_GAMEPAD_AXIS_LEFTX);

            m_CurrentState.Gamepad.sThumbLY =
                (short)SDL_GetGamepadAxis(m_SDLController, SDL_GAMEPAD_AXIS_LEFTY);
        }

#else

        // XINPUT
        DWORD result = XInputGetState(0, &m_CurrentState);
        if (result != ERROR_SUCCESS)
            m_CurrentState = {};
#endif

        auto buttonChanges = m_CurrentState.Gamepad.wButtons ^ m_PreviousState.Gamepad.wButtons;
        m_ButtonsPressedThisFrame = buttonChanges & m_CurrentState.Gamepad.wButtons;
        m_ButtonsReleasedThisFrame = buttonChanges & (~m_CurrentState.Gamepad.wButtons);

        for (auto& [button, command] : m_ButtonBindings)
        {
            if (IsDownThisFrame(button) && command)
                command->Execute();
        }

        return true;
    }

    float InputManager::GetLeftStickX() const
    {
        const float deadzone = 0.05f;
        float value = m_CurrentState.Gamepad.sThumbLX / 32767.f;
        return (fabs(value) < deadzone) ? 0.f : value;
    }

    float InputManager::GetLeftStickY() const
    {
        const float deadzone = 0.05f;
        float value = m_CurrentState.Gamepad.sThumbLY / 32767.f;
        return (fabs(value) < deadzone) ? 0.f : value;
    }

    bool InputManager::IsDownThisFrame(WORD button) const
    {
        return (m_ButtonsPressedThisFrame & button) != 0;
    }

    bool InputManager::IsUpThisFrame(WORD button) const
    {
        return (m_ButtonsReleasedThisFrame & button) != 0;
    }

    bool InputManager::IsPressed(WORD button) const
    {
        return (m_CurrentState.Gamepad.wButtons & button) != 0;
    }

    void InputManager::BindCommand(WORD button, std::unique_ptr<Command> command)
    {
        m_ButtonBindings[button] = std::move(command);
    }
}