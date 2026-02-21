#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "Scene.h"
#include "RenderComponent.h"
#include "Transform.h"
#include "TextComponent.h"
#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
    // Get the scene
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    // ======================
    // Background GameObject
    // ======================
    auto backgroundGO = std::make_unique<dae::GameObject>();

    // Transform
    auto bgTransform = std::make_unique<dae::Transform>(*backgroundGO);
    bgTransform->SetPosition(0, 0);
    backgroundGO->AddComponent(std::move(bgTransform));

    // Render Component
    auto bgRender = std::make_unique<dae::RenderComponent>(*backgroundGO);
    bgRender->SetTexture("background.png");
    backgroundGO->AddComponent(std::move(bgRender));

    // Add to scene
    scene.Add(std::move(backgroundGO));

    // Logo GameObject
    auto logoGO = std::make_unique<dae::GameObject>();

    auto logoTransform = std::make_unique<dae::Transform>(*logoGO);
    logoTransform->SetPosition(358, 180);
    logoGO->AddComponent(std::move(logoTransform));

    auto logoRender = std::make_unique<dae::RenderComponent>(*logoGO);
    logoRender->SetTexture("logo.png");
    logoGO->AddComponent(std::move(logoRender));

    scene.Add(std::move(logoGO));

    // Text GameObject
    auto textGO = std::make_unique<dae::GameObject>();

    auto textTransform = std::make_unique<dae::Transform>(*textGO);
    textTransform->SetPosition(292, 20);
    textGO->AddComponent(std::move(textTransform));

    auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);

    auto textComponent = std::make_unique<dae::TextComponent>(
        *textGO,                         // Owner
        "Programming 4 Assignment",
        font,
        SDL_Color{ 255, 255, 255, 255 }
    );
    textGO->AddComponent(std::move(textComponent));

    scene.Add(std::move(textGO));

   
    // FPS Display GameObject
    auto fpsTextGO = std::make_unique<dae::GameObject>();

    // Transform
    auto fpsTransform = std::make_unique<dae::Transform>(*fpsTextGO);
    fpsTransform->SetPosition(10, 10);
    fpsTextGO->AddComponent(std::move(fpsTransform));

    // FPS Text Component
    auto fpsTextComponent = std::make_unique<dae::TextComponent>(
        *fpsTextGO,
        "0 FPS",
        font,
        SDL_Color{ 255, 255, 255, 255 }
    );
    fpsTextGO->AddComponent(std::move(fpsTextComponent));

    // FPS Updater Component
    auto fpsComponent = std::make_unique<dae::FPSComponent>(*fpsTextGO);
    fpsTextGO->AddComponent(std::move(fpsComponent));

    scene.Add(std::move(fpsTextGO));
}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
    return 0;
}
