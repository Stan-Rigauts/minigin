#pragma once
#include "Singleton.h"
#include "Command.h"
#include <windows.h>
#include <Xinput.h>
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

		// Bind a command to a button
		void BindCommand(WORD button, std::unique_ptr<Command> command);

	private:
		XINPUT_STATE m_CurrentState{};
		XINPUT_STATE m_PreviousState{};

		WORD m_ButtonsPressedThisFrame{};
		WORD m_ButtonsReleasedThisFrame{};

		// Stores button → command mapping
		std::map<WORD, std::unique_ptr<Command>> m_ButtonBindings{};
	};
}
