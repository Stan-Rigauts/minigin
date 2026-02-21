#pragma once
#include <string>
#include <functional>
#include <filesystem>

namespace dae
{
	class Minigin final
	{
		bool m_quit{};
	public:
		explicit Minigin(const std::filesystem::path& dataPath);
		~Minigin();
		void Run(const std::function<void()>& load);

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;
	private:
		const int target_fps = 60;
		const int ms_per_frame = 1000 / target_fps;  

		void RunOneFrame();
#ifdef __EMSCRIPTEN__
		static void LoopCallback(void* arg);
#endif
	};
}