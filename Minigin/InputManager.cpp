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
        XINPUT_STATE m_CurrentState{};
        XINPUT_STATE m_PreviousState{};

        WORD m_ButtonsPressedThisFrame{};
        WORD m_ButtonsReleasedThisFrame{};

        std::vector<bool> m_PreviousKeyState;

        std::unordered_map<SDL_Scancode,
            std::pair<std::unique_ptr<Command>, InputTriggerType>> m_KeyBindings;

        std::unordered_map<WORD,
            std::pair<std::unique_ptr<Command>, InputTriggerType>> m_ButtonBindings;

#ifdef __EMSCRIPTEN__
        SDL_ControllerType* m_SDLController = nullptr;
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

        // --- KEYBOARD ---
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

        // Store previous keyboard state
        for (int i = 0; i < numKeys; ++i)
            m_pImpl->m_PreviousKeyState[i] = keyState[i];

        // --- CONTROLLER ---
        m_pImpl->m_PreviousState = m_pImpl->m_CurrentState;
        m_pImpl->m_CurrentState = {};

#ifdef __EMSCRIPTEN__
        if (!m_pImpl->m_SDLController)
        {
            int count = 0;
            SDL_JoystickID* joysticks = SDL_GetJoysticks(&count);
            if (count > 0)
                if (SDL_IsGamepad(joysticks[0]))
                    m_pImpl->m_SDLController = SDL_OpenGamepad(joysticks[0]);
            SDL_free(joysticks);
        }

        if (m_pImpl->m_SDLController)
        {
            WORD buttons = 0;
            for (int b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; ++b)
                if (SDL_GetGamepadButton(m_pImpl->m_SDLController, (SDL_GamepadButton)b))
                    buttons |= (1 << b);

            m_pImpl->m_CurrentState.Gamepad.wButtons = buttons;
            m_pImpl->m_CurrentState.Gamepad.sThumbLX =
                (short)SDL_GetGamepadAxis(m_pImpl->m_SDLController, SDL_GAMEPAD_AXIS_LEFTX);
            m_pImpl->m_CurrentState.Gamepad.sThumbLY =
                (short)SDL_GetGamepadAxis(m_pImpl->m_SDLController, SDL_GAMEPAD_AXIS_LEFTY);
        }
#else
        DWORD result = XInputGetState(0, &m_pImpl->m_CurrentState);
        if (result != ERROR_SUCCESS)
            m_pImpl->m_CurrentState = {};
#endif

        auto buttonChanges =
            m_pImpl->m_CurrentState.Gamepad.wButtons ^
            m_pImpl->m_PreviousState.Gamepad.wButtons;

        m_pImpl->m_ButtonsPressedThisFrame =
            buttonChanges & m_pImpl->m_CurrentState.Gamepad.wButtons;

        m_pImpl->m_ButtonsReleasedThisFrame =
            buttonChanges & (~m_pImpl->m_CurrentState.Gamepad.wButtons);

        for (auto& [button, pair] : m_pImpl->m_ButtonBindings)
        {
            auto& [command, trigger] = pair;
            ControllerButton btn = static_cast<ControllerButton>(button);

            switch (trigger)
            {
            case InputTriggerType::WhilePressed:
                if (IsPressed(btn)) command->Execute();
                break;

            case InputTriggerType::OnPressed:
                if (IsDownThisFrame(btn)) command->Execute();
                break;

            case InputTriggerType::OnReleased:
                if (IsUpThisFrame(btn)) command->Execute();
                break;
            }
        }

        return true;
    }

    float InputManager::GetLeftStickX() const
    {
        const float deadzone = 0.05f;
        float value = m_pImpl->m_CurrentState.Gamepad.sThumbLX / 32767.f;
        return (fabs(value) < deadzone) ? 0.f : value;
    }

    float InputManager::GetLeftStickY() const
    {
        const float deadzone = 0.05f;
        float value = m_pImpl->m_CurrentState.Gamepad.sThumbLY / 32767.f;
        return (fabs(value) < deadzone) ? 0.f : value;
    }

    bool InputManager::IsDownThisFrame(ControllerButton button) const
    {
        return (m_pImpl->m_ButtonsPressedThisFrame &
            static_cast<unsigned short>(button)) != 0;
    }

    bool InputManager::IsUpThisFrame(ControllerButton button) const
    {
        return (m_pImpl->m_ButtonsReleasedThisFrame &
            static_cast<unsigned short>(button)) != 0;
    }

    bool InputManager::IsPressed(ControllerButton button) const
    {
        return (m_pImpl->m_CurrentState.Gamepad.wButtons &
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
        InputTriggerType trigger)
    {
        m_pImpl->m_ButtonBindings[static_cast<unsigned short>(button)] =
        { std::move(command), trigger };
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