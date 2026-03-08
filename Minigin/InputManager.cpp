#include "InputManager.h"
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>

namespace dae
{
	bool InputManager::ProcessInput()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT)
				return false;

			ImGui_ImplSDL3_ProcessEvent(&e);
		}

		// Update controller state
		m_PreviousState = m_CurrentState;
		m_CurrentState = {};
		DWORD result = XInputGetState(0, &m_CurrentState);

		if (result != ERROR_SUCCESS)
			m_CurrentState = {};

		auto buttonChanges = m_CurrentState.Gamepad.wButtons ^ m_PreviousState.Gamepad.wButtons;
		m_ButtonsPressedThisFrame = buttonChanges & m_CurrentState.Gamepad.wButtons;
		m_ButtonsReleasedThisFrame = buttonChanges & (~m_CurrentState.Gamepad.wButtons);


		// Execute commands bound to pressed buttons
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
		return (abs(value) < deadzone) ? 0.f : value;
	}

	float InputManager::GetLeftStickY() const
	{
		const float deadzone = 0.05f;
		float value = m_CurrentState.Gamepad.sThumbLY / 32767.f;
		return (abs(value) < deadzone) ? 0.f : value;
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
