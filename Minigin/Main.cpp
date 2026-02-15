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
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    auto backgroundGO = std::make_unique<dae::GameObject>();

    auto bgTransform = std::make_unique<dae::Transform>();
    bgTransform->SetPosition(0, 0);
    backgroundGO->AddComponent(std::move(bgTransform));


    auto bgRender = std::make_unique<dae::RenderComponent>(backgroundGO.get());
    bgRender->SetTexture("background.png");
    backgroundGO->AddComponent(std::move(bgRender));

    scene.Add(std::move(backgroundGO));

    auto logoGO = std::make_unique<dae::GameObject>();

    auto logoTransform = std::make_unique<dae::Transform>();
    logoTransform->SetPosition(358, 180);
    logoGO->AddComponent(std::move(logoTransform));

    auto logoRender = std::make_unique<dae::RenderComponent>(logoGO.get());
    logoRender->SetTexture("logo.png");
    logoGO->AddComponent(std::move(logoRender));
    scene.Add(std::move(logoGO));

    auto textGO = std::make_unique<dae::GameObject>();

    auto textTransform = std::make_unique<dae::Transform>();
    textTransform->SetPosition(292, 20);
    textGO->AddComponent(std::move(textTransform));


    auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);

    auto textComponent = std::make_unique<dae::TextComponent>(
        "Programming 4 Assignment",
        font,
        SDL_Color{ 255, 255, 255, 255 }
    );


    textGO->AddComponent(std::move(textComponent));

    scene.Add(std::move(textGO));


    auto fpsText = std::make_unique<dae::GameObject>();
    auto fpstextComponent = std::make_unique<dae::TextComponent>(
        "0 FPS",
        font,
        SDL_Color{ 255, 255, 255, 255 }
    );

    fpsText->AddComponent(std::move(fpstextComponent));
    auto fpsTransform = std::make_unique<dae::Transform>();
    fpsTransform->SetPosition(10, 10);
    fpsText->AddComponent(std::move(fpsTransform));
    auto fpsTextComponent = std::make_unique<dae::FPSComponent>();
    fpsText->AddComponent(std::move(fpsTextComponent));
    scene.Add(std::move(fpsText));
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
