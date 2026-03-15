#pragma once
#include "Singleton.h"
#include "Command.h"
#include <memory>
#include <SDL3/SDL.h>

namespace dae
{
    enum class ControllerButton : unsigned short
    {
        DPadUp = 0x0001,
        DPadDown = 0x0002,
        DPadLeft = 0x0004,
        DPadRight = 0x0008,
        Start = 0x0010,
        Back = 0x0020,
        LeftThumb = 0x0040,
        RightThumb = 0x0080,
        LeftShoulder = 0x0100,
        RightShoulder = 0x0200,
        A = 0x1000,
        B = 0x2000,
        X = 0x4000,
        Y = 0x8000,
    };

    class InputManager final : public Singleton<InputManager>
    {
    public:
        InputManager();
        ~InputManager();
        bool ProcessInput();
        float GetLeftStickX() const;
        float GetLeftStickY() const;
        bool IsDownThisFrame(ControllerButton button) const;
        bool IsUpThisFrame(ControllerButton button) const;
        bool IsPressed(ControllerButton button) const;
        void BindCommand(ControllerButton button, std::unique_ptr<Command> command);
        void BindCommand(SDL_Scancode key, std::unique_ptr<Command> command);
        void UnbindCommand(SDL_Scancode key);
        void UnbindCommand(ControllerButton button);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}