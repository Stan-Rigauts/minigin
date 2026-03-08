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
#include "RotatorComponent.h"
#include "Transform.h"
#include "TextComponent.h"
#include "ThrashTheCacheComponent.h"
#include "PlayerControllerComponent.h"
#include "MoveComponent.h"
#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	// Background
	auto backgroundGO = std::make_unique<dae::GameObject>();
	backgroundGO->SetLocalPosition(0, 0);
	auto bgRender = std::make_unique<dae::RenderComponent>(*backgroundGO);
	bgRender->SetTexture("background.png");
	backgroundGO->AddComponent(std::move(bgRender));
	scene.Add(std::move(backgroundGO));

	// Logo
	auto logoGO = std::make_unique<dae::GameObject>();
	logoGO->SetLocalPosition(358, 180);
	auto logoRender = std::make_unique<dae::RenderComponent>(*logoGO);
	logoRender->SetTexture("logo.png");
	logoGO->AddComponent(std::move(logoRender));
	scene.Add(std::move(logoGO));

	// Title text
	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto textGO = std::make_unique<dae::GameObject>();
	textGO->SetLocalPosition(292, 20);
	auto textComponent = std::make_unique<dae::TextComponent>(
		*textGO, "Programming 4 Assignment", font, SDL_Color{ 255, 255, 255, 255 }
	);
	textGO->AddComponent(std::move(textComponent));
	scene.Add(std::move(textGO));

	
	auto fpsTextGO = std::make_unique<dae::GameObject>();
	fpsTextGO->SetLocalPosition(10, 10);
	auto fpsTextComponent = std::make_unique<dae::TextComponent>(
		*fpsTextGO, "0 FPS", font, SDL_Color{ 255, 255, 255, 255 }
	);
	fpsTextGO->AddComponent(std::move(fpsTextComponent));
	auto fpsComponent = std::make_unique<dae::FPSComponent>(*fpsTextGO);
	fpsTextGO->AddComponent(std::move(fpsComponent));
	scene.Add(std::move(fpsTextGO));


	auto rotator = std::make_unique<dae::GameObject>();
	rotator->SetLocalPosition(350, 300);

	auto parent = std::make_unique<dae::GameObject>();
	
	parent->SetParent(rotator.get(), false);
	auto parentRender = std::make_unique<dae::RenderComponent>(*parent);
	parentRender->SetTexture("MSPAC.png");
	parent->AddComponent(std::move(parentRender));
	parent->AddComponent(std::make_unique<dae::RotatorComponent>(*parent, 25.f, 3.f  ));

	auto child = std::make_unique<dae::GameObject>();
	child->SetParent(parent.get(), false);                    
	auto childRender = std::make_unique<dae::RenderComponent>(*child);
	childRender->SetTexture("MSPAC.png");
	child->AddComponent(std::move(childRender));
	child->AddComponent(std::make_unique<dae::RotatorComponent>(
		*child, 75.f, -2.f                               
	));



	// --- Create Player ---
	auto player = std::make_unique<dae::GameObject>();

	// Render
	auto render = std::make_unique<dae::RenderComponent>(*player);
	render->SetTexture("MSPAC.png");
	player->AddComponent(std::move(render));

	// MoveComponent
	auto moveComp = std::make_unique<dae::MoveComponent>(*player, 200.f); // 200 units per second
	player->AddComponent(std::move(moveComp));

	// PlayerControllerComponent reads input and moves the player
	player->AddComponent(std::make_unique<dae::PlayerControllerComponent>(*player));

	// Add player to the scene
	scene.Add(std::move(player));



	//auto imguitrasher = std::make_unique<dae::GameObject>();
	//auto trasher = std::make_unique<dae::ThrashTheCacheComponent>(*imguitrasher);
	//imguitrasher->AddComponent(std::move(trasher));


	scene.Add(std::move(rotator));
	scene.Add(std::move(parent));
	scene.Add(std::move(child));
	//scene.Add(std::move(imguitrasher));
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
