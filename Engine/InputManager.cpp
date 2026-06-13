#include "InputManager.h"
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <cmath>
#include <vector>

#ifdef __EMSCRIPTEN__
typedef unsigned short WORD;
typedef unsigned long DWORD;
struct XINPUT_GAMEPAD { WORD wButtons = 0; short sThumbLX = 0; short sThumbLY = 0; };
struct XINPUT_STATE { XINPUT_GAMEPAD Gamepad; };
typedef SDL_Gamepad SDL_ControllerType;
#else
#include <windows.h>
#include <Xinput.h>
typedef void SDL_ControllerType;
#endif

namespace dae
{
    struct InputManager::Impl
    {
        XINPUT_STATE m_CurrentState[2]{};
        XINPUT_STATE m_PreviousState[2]{};
        WORD         m_ButtonsPressedThisFrame[2]{};
        WORD         m_ButtonsReleasedThisFrame[2]{};
        int m_ClaimedXInputIndices[2]{ -1, -1 };
        bool m_ControllerClaimed[2]{ false, false };
        std::vector<bool> m_PreviousKeyState;

        std::unordered_map<SDL_Scancode,
            std::pair<std::unique_ptr<Command>, InputTriggerType>> m_KeyBindings;

        std::unordered_map<WORD,
            std::tuple<std::unique_ptr<Command>, InputTriggerType, int>> m_ButtonBindings;

#ifdef __EMSCRIPTEN__
        SDL_ControllerType* m_SDLController[2]{ nullptr, nullptr };
#endif
    };

    InputManager::InputManager()
        : m_pImpl(std::make_unique<Impl>())
    {
        int numKeys = 0;
        SDL_GetKeyboardState(&numKeys);
        m_pImpl->m_PreviousKeyState.resize(numKeys, false);
    }

    InputManager::~InputManager() = default;

    bool InputManager::ProcessInput()
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
                return false;
            ImGui_ImplSDL3_ProcessEvent(&e);
        }

        int numKeys = 0;
        const bool* keyState = SDL_GetKeyboardState(&numKeys);

        for (auto& [scancode, pair] : m_pImpl->m_KeyBindings)
        {
            auto& [command, trigger] = pair;

            bool isDownNow = keyState[scancode];
            bool wasDownBefore = m_pImpl->m_PreviousKeyState[scancode];

            switch (trigger)
            {
            case InputTriggerType::WhilePressed:
                if (isDownNow) command->Execute();
                break;
            case InputTriggerType::OnPressed:
                if (isDownNow && !wasDownBefore) command->Execute();
                break;
            case InputTriggerType::OnReleased:
                if (!isDownNow && wasDownBefore) command->Execute();
                break;
            }
        }

        for (int i = 0; i < numKeys; ++i)
            m_pImpl->m_PreviousKeyState[i] = keyState[i];

#ifdef __EMSCRIPTEN__
        int count = 0;
        SDL_JoystickID* joysticks = SDL_GetJoysticks(&count);
        for (int i = 0; i < 2 && i < count; ++i)
        {
            if (!m_pImpl->m_SDLController[i] && SDL_IsGamepad(joysticks[i]))
                m_pImpl->m_SDLController[i] = SDL_OpenGamepad(joysticks[i]);

            m_pImpl->m_PreviousState[i] = m_pImpl->m_CurrentState[i];
            m_pImpl->m_CurrentState[i] = {};

            if (m_pImpl->m_SDLController[i])
            {
                WORD buttons = 0;
                for (int b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; ++b)
                    if (SDL_GetGamepadButton(m_pImpl->m_SDLController[i], (SDL_GamepadButton)b))
                        buttons |= (1 << b);

                m_pImpl->m_CurrentState[i].Gamepad.wButtons = buttons;
                m_pImpl->m_CurrentState[i].Gamepad.sThumbLX =
                    (short)SDL_GetGamepadAxis(m_pImpl->m_SDLController[i], SDL_GAMEPAD_AXIS_LEFTX);
                m_pImpl->m_CurrentState[i].Gamepad.sThumbLY =
                    (short)SDL_GetGamepadAxis(m_pImpl->m_SDLController[i], SDL_GAMEPAD_AXIS_LEFTY);
            }

            auto buttonChanges = m_pImpl->m_CurrentState[i].Gamepad.wButtons ^
                m_pImpl->m_PreviousState[i].Gamepad.wButtons;
            m_pImpl->m_ButtonsPressedThisFrame[i] = buttonChanges & m_pImpl->m_CurrentState[i].Gamepad.wButtons;
            m_pImpl->m_ButtonsReleasedThisFrame[i] = buttonChanges & ~m_pImpl->m_CurrentState[i].Gamepad.wButtons;
        }
        SDL_free(joysticks);
#else
        // Auto-claim controllers on first input (skips idle pads like DS4Windows virtual device)
        for (int xi = 0; xi < 4; ++xi)
        {
            bool alreadyClaimed = false;
            for (int s = 0; s < 2; ++s)
                if (m_pImpl->m_ClaimedXInputIndices[s] == xi) { alreadyClaimed = true; break; }
            if (alreadyClaimed) continue;

            XINPUT_STATE state{};
            if (XInputGetState(xi, &state) != ERROR_SUCCESS) continue;

            bool anyInput = state.Gamepad.wButtons != 0
                || abs(state.Gamepad.sThumbLX) > 8000
                || abs(state.Gamepad.sThumbLY) > 8000;

            if (anyInput)
            {
                for (int s = 0; s < 2; ++s)
                {
                    if (m_pImpl->m_ClaimedXInputIndices[s] == -1)
                    {
                        m_pImpl->m_ClaimedXInputIndices[s] = xi;
                        break;
                    }
                }
            }
        }

        for (int slot = 0; slot < 2; ++slot)
        {
            int xi = m_pImpl->m_ClaimedXInputIndices[slot];
            if (xi == -1) continue;

            m_pImpl->m_PreviousState[slot] = m_pImpl->m_CurrentState[slot];
            m_pImpl->m_CurrentState[slot] = {};

            if (XInputGetState(xi, &m_pImpl->m_CurrentState[slot]) != ERROR_SUCCESS)
                m_pImpl->m_CurrentState[slot] = {};

            auto buttonChanges = m_pImpl->m_CurrentState[slot].Gamepad.wButtons ^
                m_pImpl->m_PreviousState[slot].Gamepad.wButtons;
            m_pImpl->m_ButtonsPressedThisFrame[slot] = buttonChanges & m_pImpl->m_CurrentState[slot].Gamepad.wButtons;
            m_pImpl->m_ButtonsReleasedThisFrame[slot] = buttonChanges & ~m_pImpl->m_CurrentState[slot].Gamepad.wButtons;
        }
#endif

        for (auto& [button, tuple] : m_pImpl->m_ButtonBindings)
        {
            auto& [command, trigger, controllerIndex] = tuple;

            switch (trigger)
            {
            case InputTriggerType::WhilePressed:
                if (m_pImpl->m_CurrentState[controllerIndex].Gamepad.wButtons & button)
                    command->Execute();
                break;
            case InputTriggerType::OnPressed:
                if (m_pImpl->m_ButtonsPressedThisFrame[controllerIndex] & button)
                    command->Execute();
                break;
            case InputTriggerType::OnReleased:
                if (m_pImpl->m_ButtonsReleasedThisFrame[controllerIndex] & button)
                    command->Execute();
                break;
            }
        }

        return true;
    }

    float InputManager::GetLeftStickX(int controllerIndex) const
    {
        const float deadzone = 0.05f;
        float value = m_pImpl->m_CurrentState[controllerIndex].Gamepad.sThumbLX / 32767.f;
        return (fabs(value) < deadzone) ? 0.f : value;
    }

    float InputManager::GetLeftStickY(int controllerIndex) const
    {
        const float deadzone = 0.05f;
        float value = m_pImpl->m_CurrentState[controllerIndex].Gamepad.sThumbLY / 32767.f;
        return (fabs(value) < deadzone) ? 0.f : value;
    }

    bool InputManager::IsDownThisFrame(ControllerButton button, int controllerIndex) const
    {
        return (m_pImpl->m_ButtonsPressedThisFrame[controllerIndex] &
            static_cast<unsigned short>(button)) != 0;
    }

    bool InputManager::IsUpThisFrame(ControllerButton button, int controllerIndex) const
    {
        return (m_pImpl->m_ButtonsReleasedThisFrame[controllerIndex] &
            static_cast<unsigned short>(button)) != 0;
    }

    bool InputManager::IsPressed(ControllerButton button, int controllerIndex) const
    {
        return (m_pImpl->m_CurrentState[controllerIndex].Gamepad.wButtons &
            static_cast<unsigned short>(button)) != 0;
    }

    void InputManager::BindCommand(SDL_Scancode key,
        std::unique_ptr<Command> command,
        InputTriggerType trigger)
    {
        m_pImpl->m_KeyBindings[key] = { std::move(command), trigger };
    }

    void InputManager::BindCommand(ControllerButton button,
        std::unique_ptr<Command> command,
        InputTriggerType trigger,
        int controllerIndex)
    {
        m_pImpl->m_ButtonBindings[static_cast<unsigned short>(button)] =
        { std::move(command), trigger, controllerIndex };
    }

    void InputManager::UnbindCommand(SDL_Scancode key)
    {
        m_pImpl->m_KeyBindings.erase(key);
    }

    void InputManager::UnbindCommand(ControllerButton button)
    {
        m_pImpl->m_ButtonBindings.erase(static_cast<unsigned short>(button));
    }
}