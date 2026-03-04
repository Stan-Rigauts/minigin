#include <stdexcept>
#include <cstring>
#include <iostream>
#include "Renderer.h"
#include "SceneManager.h"
#include "Texture2D.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "imgui_plot.h"
#include "CacheTrasher.h"
void dae::Renderer::Init(SDL_Window* window)
{
	m_window = window;
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	m_renderer = SDL_CreateRenderer(window, nullptr);
	if (m_renderer == nullptr)
	{
		std::cout << "Failed to create the renderer: " << SDL_GetError() << "\n";
		throw std::runtime_error(std::string("SDL_CreateRenderer Error: ") + SDL_GetError());
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
#if __EMSCRIPTEN__
	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	io.IniFilename = NULL;
#endif

	ImGui_ImplSDL3_InitForSDLRenderer(window, m_renderer);
	ImGui_ImplSDLRenderer3_Init(m_renderer);
}


void dae::Renderer::Render() const
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    static int exercise1Samples = 10;
    static int exercise2Samples = 100;
    static TimingResult ex1Result;
    static TimingResult ex2ResultGO3D;
    static TimingResult ex2ResultGO3DAlt;
    static bool ex1HasResult = false;
    static bool ex2HasResult = false;
    static bool ex2AltHasResult = false;

    static const float xs[11] = { 0,1,2,3,4,5,6,7,8,9,10 };

    // ── Exercise 1 ──────────────────────────────────────────────
    ImGui::Begin("Exercise 1");

    ImGui::InputInt("# samples##ex1", &exercise1Samples);
    if (exercise1Samples < 1) exercise1Samples = 1;

    if (ImGui::Button("Thrash the cache"))
    {
        ex1Result = CacheTrasher::RunExercise1(exercise1Samples);
        ex1HasResult = true;
    }

    if (ex1HasResult && !ex1Result.timings.empty())
    {
        float maxVal = *std::max_element(ex1Result.timings.begin(), ex1Result.timings.end());

        ImGui::Spacing();
        ImGui::Text("int buffer - avg us per step");

        ImGui::PlotConfig conf;
        conf.values.xs = xs;
        conf.values.ys = ex1Result.timings.data();
        conf.values.count = (int)ex1Result.timings.size();
        conf.values.color = IM_COL32(0, 200, 100, 255);
        conf.scale.min = 0.f;
        conf.scale.max = maxVal * 1.2f;
        conf.scale.type = ImGui::PlotConfig::Scale::Linear;
        conf.tooltip.show = true;
        conf.tooltip.format = "step %g : %.1f us";
        conf.grid_x.show = true;
        conf.grid_x.size = 1.f;
        conf.grid_x.subticks = 1;
        conf.grid_y.show = true;
        conf.grid_y.size = maxVal / 4.f;
        conf.grid_y.subticks = 2;
        conf.frame_size = ImVec2(ImGui::GetContentRegionAvail().x, 160);
        conf.line_thickness = 2.f;

        ImGui::Plot("##ex1plot", conf);

        ImGui::Text("x: 1  2  4  8  16  32  64  128  256  512  1024");
        for (int i = 0; i < (int)ex1Result.timings.size(); ++i)
        {
            int step = 1 << i;
            ImGui::Text("  step %4d : %.1f us", step, ex1Result.timings[i]);
        }
    }

    ImGui::End();

    // ── Exercise 2 ──────────────────────────────────────────────
    ImGui::Begin("Exercise 2");

    ImGui::InputInt("# samples##ex2", &exercise2Samples);
    if (exercise2Samples < 1) exercise2Samples = 1;

    if (ImGui::Button("Thrash the cache with GameObject3D"))
    {
        ex2ResultGO3D = CacheTrasher::RunExercise2_GameObject3D(exercise2Samples);
        ex2HasResult = true;
    }
    if (ImGui::Button("Thrash the cache with GameObject3DAlt"))
    {
        ex2ResultGO3DAlt = CacheTrasher::RunExercise2_GameObject3DAlt(exercise2Samples);
        ex2AltHasResult = true;
    }

    if (ex2HasResult || ex2AltHasResult)
    {
        ImGui::Spacing();

        float maxVal = 1.f;
        if (ex2HasResult && !ex2ResultGO3D.timings.empty())
            maxVal = std::max(maxVal, *std::max_element(ex2ResultGO3D.timings.begin(), ex2ResultGO3D.timings.end()));
        if (ex2AltHasResult && !ex2ResultGO3DAlt.timings.empty())
            maxVal = std::max(maxVal, *std::max_element(ex2ResultGO3DAlt.timings.begin(), ex2ResultGO3DAlt.timings.end()));

        if (ex2HasResult && ex2AltHasResult)
        {
            ImGui::Text("GameObject3D (green)  vs  GameObject3DAlt (orange)");

            const float* ys_list[2] = {
                ex2ResultGO3D.timings.data(),
                ex2ResultGO3DAlt.timings.data()
            };
            static const ImU32 colors[2] = {
                IM_COL32(0,   200, 100, 255),
                IM_COL32(255, 160,   0, 255)
            };

            ImGui::PlotConfig conf;
            conf.values.xs = xs;
            conf.values.ys_list = ys_list;
            conf.values.ys_count = 2;
            conf.values.colors = colors;
            conf.values.count = (int)ex2ResultGO3D.timings.size();
            conf.scale.min = 0.f;
            conf.scale.max = maxVal * 1.2f;
            conf.scale.type = ImGui::PlotConfig::Scale::Linear;
            conf.tooltip.show = true;
            conf.tooltip.format = "step %g : %.1f us";
            conf.grid_x.show = true;
            conf.grid_x.size = 1.f;
            conf.grid_x.subticks = 1;
            conf.grid_y.show = true;
            conf.grid_y.size = maxVal / 4.f;
            conf.grid_y.subticks = 2;
            conf.frame_size = ImVec2(ImGui::GetContentRegionAvail().x, 180);
            conf.line_thickness = 2.f;

            ImGui::Plot("##ex2both", conf);
        }
        else
        {
            bool useGO3D = ex2HasResult;
            const float* ys = useGO3D ? ex2ResultGO3D.timings.data() : ex2ResultGO3DAlt.timings.data();
            int cnt = useGO3D ? (int)ex2ResultGO3D.timings.size() : (int)ex2ResultGO3DAlt.timings.size();
            ImU32 col = useGO3D ? IM_COL32(0, 200, 100, 255) : IM_COL32(255, 160, 0, 255);
            const char* lbl = useGO3D ? "GameObject3D (pointer chase)" : "GameObject3DAlt (direct)";

            ImGui::Text("%s", lbl);

            ImGui::PlotConfig conf;
            conf.values.xs = xs;
            conf.values.ys = ys;
            conf.values.count = cnt;
            conf.values.color = col;
            conf.scale.min = 0.f;
            conf.scale.max = maxVal * 1.2f;
            conf.scale.type = ImGui::PlotConfig::Scale::Linear;
            conf.tooltip.show = true;
            conf.tooltip.format = "step %g : %.1f us";
            conf.grid_x.show = true;
            conf.grid_x.size = 1.f;
            conf.grid_x.subticks = 1;
            conf.grid_y.show = true;
            conf.grid_y.size = maxVal / 4.f;
            conf.grid_y.subticks = 2;
            conf.frame_size = ImVec2(ImGui::GetContentRegionAvail().x, 180);
            conf.line_thickness = 2.f;

            ImGui::Plot("##ex2single", conf);
        }

        ImGui::Text("x: 1  2  4  8  16  32  64  128  256  512  1024");

        if (ex2HasResult && !ex2ResultGO3D.timings.empty()) {
            ImGui::Text("GameObject3D:");
            for (int i = 0; i < (int)ex2ResultGO3D.timings.size(); ++i)
                ImGui::Text("  step %4d : %.1f us", 1 << i, ex2ResultGO3D.timings[i]);
        }
        if (ex2AltHasResult && !ex2ResultGO3DAlt.timings.empty()) {
            ImGui::Text("GameObject3DAlt:");
            for (int i = 0; i < (int)ex2ResultGO3DAlt.timings.size(); ++i)
                ImGui::Text("  step %4d : %.1f us", 1 << i, ex2ResultGO3DAlt.timings[i]);
        }
    }

    ImGui::End();

    ImGui::Render();
    const auto& color = GetBackgroundColor();
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
    SceneManager::GetInstance().Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
    SDL_RenderPresent(m_renderer);
}

void dae::Renderer::Destroy()
{
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	if (m_renderer != nullptr)
	{
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}
}

void dae::Renderer::RenderTexture(const Texture2D& texture, const float x, const float y) const
{
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	SDL_GetTextureSize(texture.GetSDLTexture(), &dst.w, &dst.h);
	SDL_RenderTexture(GetSDLRenderer(), texture.GetSDLTexture(), nullptr, &dst);
}

void dae::Renderer::RenderTexture(const Texture2D& texture, const float x, const float y, const float width, const float height) const
{
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	dst.w = width;
	dst.h = height;
	SDL_RenderTexture(GetSDLRenderer(), texture.GetSDLTexture(), nullptr, &dst);
}

SDL_Renderer* dae::Renderer::GetSDLRenderer() const { return m_renderer; }
