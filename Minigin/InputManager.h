#pragma once
#include "Singleton.h"
#include "Command.h"
#ifndef __EMSCRIPTEN__
#include <windows.h>
#include <Xinput.h>
#endif
#include <map>
#include <memory>

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
SDL_GameController* m_SDLController = nullptr;
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
