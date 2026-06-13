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


    // Gamemanager
    auto managerGO = std::make_unique<dae::GameObject>();
    auto managerComp = std::make_unique<dae::GameManagerComponent>(*managerGO);
    auto* manager = managerComp.get();
    managerGO->AddComponent(std::move(managerComp));
    scene.Add(std::move(managerGO));

	//sound system
    servicelocator::register_sound_system(std::make_unique<sdl_sound_system>());

    servicelocator::get_sound_system().load(SND_PELLET, "Data/Sounds/ms_eat_dot.wav");
    servicelocator::get_sound_system().load(SND_DEATH, "Data/Sounds/ms_death.wav");
    servicelocator::get_sound_system().load(SND_LEVELSTART, "Data/Sounds/ms_start.wav");
    servicelocator::get_sound_system().load(SND_GHOST_EATEN, "Data/Sounds/ms_eat_ghost.wav");
    servicelocator::get_sound_system().load(SND_RUNNING, "Data/Sounds/ms_siren.wav");

    manager->SetScene(&scene);
    manager->SetLevels(
        { "level1.txt","level2.txt","level3.txt"},
        { "Level1.png", "Level2.png", "Level3.png" });

    manager->BuildStartScreen();
}

int main(int, char* [])
{
    fs::path data_location = fs::absolute("Data");

    if (!fs::exists(data_location))
        data_location = "./Data/";

    dae::Minigin engine(data_location);
    engine.Run(load);

    return 0;
}