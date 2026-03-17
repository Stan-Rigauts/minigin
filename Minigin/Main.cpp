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
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "PlayerControllerComponent.h"
#include "LivesDisplayComponent.h"
#include "ScoreDisplayComponent.h"
#include "MoveComponent.h"
#include <filesystem>
#include "InputManager.h"


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



	// Player 1 (keyboard)
	auto player = std::make_unique<dae::GameObject>();

	auto render = std::make_unique<dae::RenderComponent>(*player);
	render->SetTexture("MSPAC.png");
	player->AddComponent(std::move(render));

	auto moveComp = std::make_unique<dae::MoveComponent>(*player, 200.f);
	player->AddComponent(std::move(moveComp));

	// --- Add Health ---
	auto healthComp = std::make_unique<dae::HealthComponent>(*player, 3);
	auto* healthPtr = healthComp.get();
	player->AddComponent(std::move(healthComp));

	// --- Add Score ---
	auto scoreComp = std::make_unique<dae::ScoreComponent>(*player, 0);
	auto* scorePtr = scoreComp.get();
	player->AddComponent(std::move(scoreComp));

	// --- Bind Commands ---
	auto& input = dae::InputManager::GetInstance();
	input.BindCommand(SDL_SCANCODE_W, std::make_unique<dae::MoveCommand>(player.get(), 0.f, 1.f),dae::InputTriggerType::WhilePressed);
	input.BindCommand(SDL_SCANCODE_S, std::make_unique<dae::MoveCommand>(player.get(), 0.f, -1.f), dae::InputTriggerType::WhilePressed);
	input.BindCommand(SDL_SCANCODE_A, std::make_unique<dae::MoveCommand>(player.get(), -1.f, 0.f), dae::InputTriggerType::WhilePressed);
	input.BindCommand(SDL_SCANCODE_D, std::make_unique<dae::MoveCommand>(player.get(), 1.f, 0.f), dae::InputTriggerType::WhilePressed);

	// Hurt yourself
	input.BindCommand(SDL_SCANCODE_C, std::make_unique<dae::HealthCommand>(healthPtr, 1), dae::InputTriggerType::OnPressed);

	// Give score
	input.BindCommand(SDL_SCANCODE_Z, std::make_unique<dae::ScoreCommand>(scorePtr, 100), dae::InputTriggerType::OnPressed);
	input.BindCommand(SDL_SCANCODE_X, std::make_unique<dae::ScoreCommand>(scorePtr, 200), dae::InputTriggerType::OnPressed);

	scene.Add(std::move(player));

	auto p1LivesGO = std::make_unique<dae::GameObject>();
	p1LivesGO->SetLocalPosition(10, 60);

	auto p1LivesText = std::make_unique<dae::TextComponent>(
		*p1LivesGO, "Lives: 3", font, SDL_Color{ 255,255,255,255 }
	);
	auto* p1LivesPtr = p1LivesText.get();
	p1LivesGO->AddComponent(std::move(p1LivesText));

	p1LivesGO->AddComponent(std::make_unique<dae::LivesDisplayComponent>(
		*p1LivesGO, *healthPtr, *p1LivesPtr
	));

	scene.Add(std::move(p1LivesGO));

	auto p1ScoreGO = std::make_unique<dae::GameObject>();
	p1ScoreGO->SetLocalPosition(10, 90);   

	auto p1ScoreText = std::make_unique<dae::TextComponent>(
		*p1ScoreGO, "Score: 0", font, SDL_Color{ 255,255,255,255 }
	);
	auto* p1ScorePtr = p1ScoreText.get();
	p1ScoreGO->AddComponent(std::move(p1ScoreText));

	p1ScoreGO->AddComponent(std::make_unique<dae::ScoreDisplayComponent>(
		*p1ScoreGO, *scorePtr, *p1ScorePtr
	));

	scene.Add(std::move(p1ScoreGO));




	// Player 2 (controller)
	auto player2 = std::make_unique<dae::GameObject>();

	auto render2 = std::make_unique<dae::RenderComponent>(*player2);
	render2->SetTexture("MSPAC.png");
	player2->AddComponent(std::move(render2));

	auto moveComp2 = std::make_unique<dae::MoveComponent>(*player2, 400.f);
	player2->AddComponent(std::move(moveComp2));
	player2->AddComponent(std::make_unique<dae::PlayerControllerComponent>(*player2));

	// --- Add Health ---
	auto healthComp2 = std::make_unique<dae::HealthComponent>(*player2, 3);
	auto* health2Ptr = healthComp2.get();
	player2->AddComponent(std::move(healthComp2));

	// --- Add Score ---
	auto scoreComp2 = std::make_unique<dae::ScoreComponent>(*player2, 0);
	auto* score2Ptr = scoreComp2.get();
	player2->AddComponent(std::move(scoreComp2));

	// --- Bind Controller Commands ---
	input.BindCommand(dae::ControllerButton::DPadUp, std::make_unique<dae::MoveCommand>(player2.get(), 0.f, 1.f), dae::InputTriggerType::WhilePressed);
	input.BindCommand(dae::ControllerButton::DPadDown, std::make_unique<dae::MoveCommand>(player2.get(), 0.f, -1.f), dae::InputTriggerType::WhilePressed);
	input.BindCommand(dae::ControllerButton::DPadLeft, std::make_unique<dae::MoveCommand>(player2.get(), -1.f, 0.f), dae::InputTriggerType::WhilePressed);
	input.BindCommand(dae::ControllerButton::DPadRight, std::make_unique<dae::MoveCommand>(player2.get(), 1.f, 0.f), dae::InputTriggerType::WhilePressed);

	// Hurt yourself (controller)
	input.BindCommand(dae::ControllerButton::A, std::make_unique<dae::HealthCommand>(health2Ptr, 1), dae::InputTriggerType::OnPressed);

	// Give score (controller)
	input.BindCommand(dae::ControllerButton::B, std::make_unique<dae::ScoreCommand>(score2Ptr, 100), dae::InputTriggerType::OnPressed);


	auto p2LivesGO = std::make_unique<dae::GameObject>();
	p2LivesGO->SetLocalPosition(10, 120);   // Lives offset

	auto p2LivesText = std::make_unique<dae::TextComponent>(
		*p2LivesGO, "Lives: 3", font, SDL_Color{ 255,255,255,255 }
	);
	auto* p2LivesPtr = p2LivesText.get();
	p2LivesGO->AddComponent(std::move(p2LivesText));

	p2LivesGO->AddComponent(std::make_unique<dae::LivesDisplayComponent>(
		*p2LivesGO, *health2Ptr, *p2LivesPtr
	));

	scene.Add(std::move(p2LivesGO));

	auto p2ScoreGO = std::make_unique<dae::GameObject>();
	p2ScoreGO->SetLocalPosition(10, 150);   // Score offset (different!)

	auto p2ScoreText = std::make_unique<dae::TextComponent>(
		*p2ScoreGO, "Score: 0", font, SDL_Color{ 255,255,255,255 }
	);
	auto* p2ScorePtr = p2ScoreText.get();
	p2ScoreGO->AddComponent(std::move(p2ScoreText));

	p2ScoreGO->AddComponent(std::make_unique<dae::ScoreDisplayComponent>(
		*p2ScoreGO, *score2Ptr, *p2ScorePtr
	));

	scene.Add(std::move(p2ScoreGO));

	scene.Add(std::move(player2));

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
