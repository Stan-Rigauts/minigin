#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "FPSComponent.h"
#include "TextComponent.h"
#include "Scene.h"
#include "GameObject.h"
#include "GameManagerComponent.h"
#include "GameState.h"
#include "servicelocator.h"
#include "sdl_sound_system.h"
#include "SoundIds.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    auto smallFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);

    // FPS counter
    auto fpsGO = std::make_unique<dae::GameObject>();
    fpsGO->SetLocalPosition(10.f, 70.f);
    fpsGO->AddComponent(std::make_unique<dae::TextComponent>(
        *fpsGO,
        "0 FPS",
        smallFont,
        SDL_Color{ 255, 255, 255, 255 }
    ));
    fpsGO->AddComponent(std::make_unique<dae::FPSComponent>(*fpsGO));
    scene.Add(std::move(fpsGO));

    // Game manager
    auto managerGO = std::make_unique<dae::GameObject>();
    auto managerComp = std::make_unique<dae::GameManagerComponent>(*managerGO);
    auto* manager = managerComp.get();
    managerGO->AddComponent(std::move(managerComp));
    scene.Add(std::move(managerGO));

    servicelocator::register_sound_system(std::make_unique<sdl_sound_system>());

#ifdef _WIN32
    std::string cwd = fs::current_path().string() + "\n";
    OutputDebugStringA(cwd.c_str());
#endif

    servicelocator::get_sound_system().load(SND_PELLET, "Data/pellet.wav");

    // Configure manager
    manager->SetScene(&scene);
    manager->SetLevels(
        { "level1.txt" },
        { "Level1.png" }
    );

    manager->StartGame(dae::GameMode::Duo);
}

int main(int, char* [])
{
    fs::path data_location = fs::absolute("Data");

    if (!fs::exists(data_location))
        data_location = "../Data/";

    dae::Minigin engine(data_location);
    engine.Run(load);

    return 0;
}