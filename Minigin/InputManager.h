#pragma once
#include "Singleton.h"
#include "Command.h"
#include <map>
#include <memory>

#ifdef __EMSCRIPTEN__

// Fake XInput types for Web
typedef unsigned short WORD;
typedef unsigned long DWORD;

struct XINPUT_GAMEPAD
{
    WORD wButtons = 0;
    short sThumbLX = 0;
    short sThumbLY = 0;
};

struct XINPUT_STATE
{
    XINPUT_GAMEPAD Gamepad;
};

// SDL3 Gamepad
#include <SDL3/SDL.h>
typedef SDL_Gamepad SDL_ControllerType;

#else


//XInput
#include <windows.h>
#include <Xinput.h>

// Dummy
typedef void SDL_ControllerType;

#endif


namespace dae
{
    class InputManager final : public Singleton<InputManager>
    {
    public:
        bool ProcessInput();

        float GetLeftStickX() const;
        float GetLeftStickY() const;

        bool IsDownThisFrame(WORD button) const;
        bool IsUpThisFrame(WORD button) const;
        bool IsPressed(WORD button) const;

        void BindCommand(WORD button, std::unique_ptr<Command> command);

        bool IsMoveUp() const { return m_MoveUp; }
        bool IsMoveDown() const { return m_MoveDown; }
        bool IsMoveLeft() const { return m_MoveLeft; }
        bool IsMoveRight() const { return m_MoveRight; }

#ifdef __EMSCRIPTEN__
        SDL_ControllerType* m_SDLController = nullptr;
#endif

    private:
        XINPUT_STATE m_CurrentState{};
        XINPUT_STATE m_PreviousState{};

        WORD m_ButtonsPressedThisFrame{};
        WORD m_ButtonsReleasedThisFrame{};

        bool m_MoveUp{ false };
        bool m_MoveDown{ false };
        bool m_MoveLeft{ false };
        bool m_MoveRight{ false };

        std::map<WORD, std::unique_ptr<Command>> m_ButtonBindings{};
    };
}