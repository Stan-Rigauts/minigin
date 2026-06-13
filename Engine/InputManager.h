#pragma once
#include "Singleton.h"
#include "BaseCommand.h"
#include <memory>
#include <SDL3/SDL.h>
#include <unordered_map>

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

    enum class InputTriggerType
    {
        OnPressed,
        OnReleased,
        WhilePressed
    };

    class InputManager final : public Singleton<InputManager>
    {
    public:
        InputManager();
        ~InputManager();

        bool ProcessInput();

        float GetLeftStickX(int controllerIndex = 0) const;
        float GetLeftStickY(int controllerIndex = 0) const;
        bool  IsDownThisFrame(ControllerButton button, int controllerIndex = 0) const;
        bool  IsUpThisFrame(ControllerButton button, int controllerIndex = 0) const;
        bool  IsPressed(ControllerButton button, int controllerIndex = 0) const;

        void BindCommand(SDL_Scancode key, std::unique_ptr<Command> command, InputTriggerType trigger);
        void BindCommand(ControllerButton button, std::unique_ptr<Command> command, InputTriggerType trigger, int controllerIndex = 0);

        void UnbindCommand(SDL_Scancode key);
        void UnbindCommand(ControllerButton button);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}