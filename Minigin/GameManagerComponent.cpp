#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "LevelLoader.h"
#include "GridComponent.h"
#include "RenderComponent.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "MoveComponent.h"
#include "PelletComponent.h"
#include "TextComponent.h"
#include "LivesDisplayComponent.h"
#include "ScoreDisplayComponent.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "PlayerControllerComponent.h"
#include "servicelocator.h"
#include "SoundIds.h"

#include <iostream>
#include <string>

dae::GameManagerComponent::GameManagerComponent(GameObject& owner)
    : Component(owner)
{
}

void dae::GameManagerComponent::SetScene(Scene* scene)
{
    m_pScene = scene;
}

void dae::GameManagerComponent::SetLevels(
    const std::vector<std::string>& levelFiles,
    const std::vector<std::string>& levelTextures)
{
    m_LevelFiles = levelFiles;
    m_LevelTextures = levelTextures;
}



void dae::GameManagerComponent::StartGame(GameMode mode)
{
    m_GameMode = mode;
    m_GameState = GameState::Playing;
    m_CurrentLevel = 0;

    ClearPlayers();
    ClearLevelObjects();
    m_pScene->RemoveObjectsWithTag(TAG_SCOREBOARD);

    LoadLevel(0);
}
    
void dae::GameManagerComponent::PelletCollected()
{
    servicelocator::get_sound_system().play(SND_PELLET, 1.0f); 
    --m_RemainingPellets;
    if (m_RemainingPellets <= 0)
        LoadNextLevel();
}

void dae::GameManagerComponent::PlayerDied(int playerIndex)
{
    if (playerIndex < 0 || playerIndex >= static_cast<int>(m_Players.size()))
        return;

    auto* health = m_Players[playerIndex].health;
    if (health->GetLives() <= 0)
    {
        bool anyAlive = false;
        for (auto& p : m_Players)
            if (p.health->GetLives() > 0) { anyAlive = true; break; }

        if (!anyAlive)
            EnterScoreBoard();
    }
}

void dae::GameManagerComponent::EnterScoreBoard()
{
    m_GameState = GameState::ScoreBoard;
    ClearLevelObjects();
    BuildScoreBoard();
}


int dae::GameManagerComponent::CountPellets(const MapInfo& map) const
{
    int count = 0;
    for (int r = 0; r < map.rows; ++r)
        for (int c = 0; c < map.cols; ++c)
            if (map.grid[r][c] == TileType::Pellet ||
                map.grid[r][c] == TileType::PowerPellet)
                ++count;
    return count;
}

void dae::GameManagerComponent::ClearLevelObjects()
{
    if (m_pScene)
        m_pScene->RemoveObjectsWithTag(TAG_LEVEL);
}

void dae::GameManagerComponent::ClearPlayers()
{
    if (m_pScene)
        m_pScene->RemoveObjectsWithTag(TAG_PLAYER);
    m_Players.clear();
}

void dae::GameManagerComponent::LoadLevel(int index)
{
    if (!m_pScene) return;

    if (index >= static_cast<int>(m_LevelFiles.size()))
    {
        EnterScoreBoard();
        return;
    }

    ClearLevelObjects();
    m_CurrentLevel = index;

    auto mapInfo = LevelLoader::LoadMap(m_LevelFiles[index]);
    m_RemainingPellets = CountPellets(mapInfo);

    const int mazePixelW = mapInfo.cols * TILE_SIZE;
    const int mazePixelH = mapInfo.rows * TILE_SIZE;
    const int mazeOffsetX = (WINDOW_W - mazePixelW) / 2;
    const int mazeOffsetY = (WINDOW_H - mazePixelH) / 2;

    //Grid
    auto gridGO = std::make_unique<GameObject>();
    gridGO->SetTag(TAG_LEVEL);
    auto gridComp = std::make_unique<GridComponent>(
        *gridGO, mapInfo, TILE_SIZE, mazeOffsetX, mazeOffsetY);
    auto* gridPtr = gridComp.get();
    gridGO->AddComponent(std::move(gridComp));
    m_pScene->Add(std::move(gridGO));

    //Maze
    auto mazeGO = std::make_unique<GameObject>();
    mazeGO->SetTag(TAG_LEVEL);
    mazeGO->SetLocalPosition(static_cast<float>(mazeOffsetX),
        static_cast<float>(mazeOffsetY));
    auto render = std::make_unique<RenderComponent>(*mazeGO);
    render->SetTexture(m_LevelTextures[index]);
    mazeGO->AddComponent(std::move(render));
    m_pScene->Add(std::move(mazeGO));





    if (m_Players.empty())
        SpawnPlayers(mapInfo, gridPtr);
    else
        ResetPlayers(mapInfo, gridPtr);

    SpawnPellets(mapInfo, gridPtr);

    std::cout << "Loaded level " << index + 1
        << " (" << m_RemainingPellets << " pellets)\n";
}

void dae::GameManagerComponent::LoadNextLevel()
{
    LoadLevel(m_CurrentLevel + 1);
}

void dae::GameManagerComponent::SpawnPlayers(const MapInfo& mapInfo, GridComponent* grid)
{
    auto& input = InputManager::GetInstance();
    auto  smallFont = ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);

    const int mazeOffsetX = (WINDOW_W - mapInfo.cols * TILE_SIZE) / 2;
    const int mazeOffsetY = (WINDOW_H - mapInfo.rows * TILE_SIZE) / 2;

    const int spawnRow = mapInfo.rows / 2;
    const int spawnCol = mapInfo.cols / 2;

    // Player 1
    {
        const float x = static_cast<float>(mazeOffsetX + (spawnCol - 1) * TILE_SIZE);
        const float y = static_cast<float>(mazeOffsetY + spawnRow * TILE_SIZE);

        auto p1 = std::make_unique<GameObject>();
        p1->SetTag(TAG_PLAYER);
        p1->SetLocalPosition(x, y);

        auto r1 = std::make_unique<RenderComponent>(*p1);
        r1->SetTexture("MSPacmanSprite.png");
        r1->SetSourceRect(0, 0, 16, 16);
        r1->SetSize(24, 24);
        p1->AddComponent(std::move(r1));

        auto moveComp1 = std::make_unique<MoveComponent>(*p1, 100.f);
        auto* movePtr1 = moveComp1.get();
        movePtr1->SetGrid(grid);
        movePtr1->SetGameManager(this);
        p1->AddComponent(std::move(moveComp1));

        auto hpComp1 = std::make_unique<HealthComponent>(*p1, 3);
        auto* hpPtr1 = hpComp1.get();
        p1->AddComponent(std::move(hpComp1));

        auto scComp1 = std::make_unique<ScoreComponent>(*p1, 0);
        auto* scPtr1 = scComp1.get();
        p1->AddComponent(std::move(scComp1));

        auto* p1Ptr = p1.get();

        m_pScene->Add(std::move(p1));

        input.BindCommand(SDL_SCANCODE_W, std::make_unique<MoveCommand>(p1Ptr, 0.f, -1.f), InputTriggerType::WhilePressed);
        input.BindCommand(SDL_SCANCODE_S, std::make_unique<MoveCommand>(p1Ptr, 0.f, 1.f), InputTriggerType::WhilePressed);
        input.BindCommand(SDL_SCANCODE_A, std::make_unique<MoveCommand>(p1Ptr, -1.f, 0.f), InputTriggerType::WhilePressed);
        input.BindCommand(SDL_SCANCODE_D, std::make_unique<MoveCommand>(p1Ptr, 1.f, 0.f), InputTriggerType::WhilePressed);
        input.BindCommand(SDL_SCANCODE_C, std::make_unique<HealthCommand>(hpPtr1, 1), InputTriggerType::OnPressed);
        input.BindCommand(SDL_SCANCODE_Z, std::make_unique<ScoreCommand>(scPtr1, 100), InputTriggerType::OnPressed);
        input.BindCommand(SDL_SCANCODE_X, std::make_unique<ScoreCommand>(scPtr1, 200), InputTriggerType::OnPressed);

        // HUD
        auto p1ScoreGO = std::make_unique<GameObject>();
        p1ScoreGO->SetTag(TAG_HUD);
        p1ScoreGO->SetLocalPosition(10, 10);
        auto p1ScoreTxt = std::make_unique<TextComponent>(*p1ScoreGO, "P1 Score: 0", smallFont, SDL_Color{ 255,255,0,255 });
        auto* p1ScoreTxtPtr = p1ScoreTxt.get();
        p1ScoreGO->AddComponent(std::move(p1ScoreTxt));
        p1ScoreGO->AddComponent(std::make_unique<ScoreDisplayComponent>(*p1ScoreGO, *scPtr1, *p1ScoreTxtPtr));
        m_pScene->Add(std::move(p1ScoreGO));

        auto p1LivesGO = std::make_unique<GameObject>();
        p1LivesGO->SetTag(TAG_HUD);
        p1LivesGO->SetLocalPosition(10, 35);
        auto p1LivesTxt = std::make_unique<TextComponent>(*p1LivesGO, "P1 Lives: 3", smallFont, SDL_Color{ 255,255,255,255 });
        auto* p1LivesTxtPtr = p1LivesTxt.get();
        p1LivesGO->AddComponent(std::move(p1LivesTxt));
        p1LivesGO->AddComponent(std::make_unique<LivesDisplayComponent>(*p1LivesGO, *hpPtr1, *p1LivesTxtPtr));
        m_pScene->Add(std::move(p1LivesGO));

        m_Players.push_back({ p1Ptr, hpPtr1, scPtr1, movePtr1, spawnRow, spawnCol - 1 });
    }

    // Player 2 (Duo or Versus only)
    if (m_GameMode == GameMode::Duo || m_GameMode == GameMode::Versus)
    {
        const float x = static_cast<float>(mazeOffsetX + (spawnCol + 1) * TILE_SIZE);
        const float y = static_cast<float>(mazeOffsetY + spawnRow * TILE_SIZE);

        auto p2 = std::make_unique<GameObject>();
        p2->SetTag(TAG_PLAYER);
        p2->SetLocalPosition(x, y);

        auto r2 = std::make_unique<RenderComponent>(*p2);
        r2->SetTexture(m_GameMode == GameMode::Versus ? "GhostSprite.png" : "MSPacmanSprite.png");
        r2->SetSourceRect(0, 0, 16, 16);
        r2->SetSize(24, 24);
        p2->AddComponent(std::move(r2));

        auto moveComp2 = std::make_unique<MoveComponent>(*p2, 100.f);
        auto* movePtr2 = moveComp2.get();
        movePtr2->SetGrid(grid);
        movePtr2->SetGameManager(this);
        p2->AddComponent(std::move(moveComp2));

        p2->AddComponent(std::make_unique<PlayerControllerComponent>(*p2));

        auto hpComp2 = std::make_unique<HealthComponent>(*p2, 3);
        auto* hpPtr2 = hpComp2.get();
        p2->AddComponent(std::move(hpComp2));

        auto scComp2 = std::make_unique<ScoreComponent>(*p2, 0);
        auto* scPtr2 = scComp2.get();
        p2->AddComponent(std::move(scComp2));

        auto* p2Ptr = p2.get();

        m_pScene->Add(std::move(p2));

        input.BindCommand(ControllerButton::DPadUp, std::make_unique<MoveCommand>(p2Ptr, 0.f, -1.f), InputTriggerType::WhilePressed);
        input.BindCommand(ControllerButton::DPadDown, std::make_unique<MoveCommand>(p2Ptr, 0.f, 1.f), InputTriggerType::WhilePressed);
        input.BindCommand(ControllerButton::DPadLeft, std::make_unique<MoveCommand>(p2Ptr, -1.f, 0.f), InputTriggerType::WhilePressed);
        input.BindCommand(ControllerButton::DPadRight, std::make_unique<MoveCommand>(p2Ptr, 1.f, 0.f), InputTriggerType::WhilePressed);
        input.BindCommand(ControllerButton::A, std::make_unique<HealthCommand>(hpPtr2, 1), InputTriggerType::OnPressed);
        input.BindCommand(ControllerButton::B, std::make_unique<ScoreCommand>(scPtr2, 100), InputTriggerType::OnPressed);
        input.BindCommand(ControllerButton::X, std::make_unique<ScoreCommand>(scPtr2, 200), InputTriggerType::OnPressed);

        // HUD
        auto p2ScoreGO = std::make_unique<GameObject>();
        p2ScoreGO->SetTag(TAG_HUD);
        p2ScoreGO->SetLocalPosition(WINDOW_W - 200, 10);
        auto p2ScoreTxt = std::make_unique<TextComponent>(*p2ScoreGO, "P2 Score: 0", smallFont, SDL_Color{ 0,255,255,255 });
        auto* p2ScoreTxtPtr = p2ScoreTxt.get();
        p2ScoreGO->AddComponent(std::move(p2ScoreTxt));
        p2ScoreGO->AddComponent(std::make_unique<ScoreDisplayComponent>(*p2ScoreGO, *scPtr2, *p2ScoreTxtPtr));
        m_pScene->Add(std::move(p2ScoreGO));

        auto p2LivesGO = std::make_unique<GameObject>();
        p2LivesGO->SetTag(TAG_HUD);
        p2LivesGO->SetLocalPosition(WINDOW_W - 200, 35);
        auto p2LivesTxt = std::make_unique<TextComponent>(*p2LivesGO, "P2 Lives: 3", smallFont, SDL_Color{ 255,255,255,255 });
        auto* p2LivesTxtPtr = p2LivesTxt.get();
        p2LivesGO->AddComponent(std::move(p2LivesTxt));
        p2LivesGO->AddComponent(std::make_unique<LivesDisplayComponent>(*p2LivesGO, *hpPtr2, *p2LivesTxtPtr));
        m_pScene->Add(std::move(p2LivesGO));

        m_Players.push_back({ p2Ptr, hpPtr2, scPtr2, movePtr2, spawnRow, spawnCol + 1 });
    }
}
void dae::GameManagerComponent::ResetPlayers(const MapInfo& mapInfo, GridComponent* grid)
{
    const int mazeOffsetX = (WINDOW_W - mapInfo.cols * TILE_SIZE) / 2;
    const int mazeOffsetY = (WINDOW_H - mapInfo.rows * TILE_SIZE) / 2;

    for (auto& entry : m_Players)
    {
        const float x = static_cast<float>(mazeOffsetX + entry.spawnCol * TILE_SIZE);
        const float y = static_cast<float>(mazeOffsetY + entry.spawnRow * TILE_SIZE);
        entry.go->SetLocalPosition(x, y);
        entry.move->SetGrid(grid);
    }
}

void dae::GameManagerComponent::SpawnPellets(const MapInfo& mapInfo, GridComponent* /*gridPtr*/)
{
    if (!m_pScene) return;

    std::vector<GameObject*> playerGOs;
    for (auto& p : m_Players)
        playerGOs.push_back(p.go);

    const int mazeOffsetX = (WINDOW_W - mapInfo.cols * TILE_SIZE) / 2;
    const int mazeOffsetY = (WINDOW_H - mapInfo.rows * TILE_SIZE) / 2;

    for (int r = 0; r < mapInfo.rows; ++r)
    {
        for (int c = 0; c < mapInfo.cols; ++c)
        {
            const TileType tile = mapInfo.grid[r][c];
            if (tile != TileType::Pellet && tile != TileType::PowerPellet)
                continue;

            const bool isPower = (tile == TileType::PowerPellet);
            const float spriteSize = isPower ? 10.f : 6.f;
            const float visualOffset = (TILE_SIZE - spriteSize) / 2.f;

            const float x = static_cast<float>(mazeOffsetX + c * TILE_SIZE) + visualOffset;
            const float y = static_cast<float>(mazeOffsetY + r * TILE_SIZE) + visualOffset;

            auto pelletGO = std::make_unique<GameObject>();
            pelletGO->SetTag(TAG_LEVEL); 
            pelletGO->SetLocalPosition(x, y);

            auto render = std::make_unique<RenderComponent>(*pelletGO);
            render->SetTexture(isPower ? "PowerPellet.png" : "Pellet.png");
            render->SetSize(spriteSize, spriteSize);
            pelletGO->AddComponent(std::move(render));

            pelletGO->AddComponent(std::make_unique<PelletComponent>(
                *pelletGO, this, playerGOs, isPower, 8.f));

            m_pScene->Add(std::move(pelletGO));
        }
    }
}


void dae::GameManagerComponent::BuildScoreBoard()
{
    if (!m_pScene) return;

    auto font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
    auto smallFont = ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);

    // Title
    auto titleGO = std::make_unique<GameObject>();
    titleGO->SetTag(TAG_SCOREBOARD);
    titleGO->SetLocalPosition(WINDOW_W / 2.f - 100, 100);
    titleGO->AddComponent(std::make_unique<TextComponent>(
        *titleGO, "GAME OVER", font, SDL_Color{ 255, 0, 0, 255 }
    ));
    m_pScene->Add(std::move(titleGO));

    for (int i = 0; i < static_cast<int>(m_Players.size()); ++i)
    {
        const int score = m_Players[i].score->GetScore();
        std::string label = (m_GameMode == GameMode::Versus && i == 1)
            ? "Ghost Score: " : ("P" + std::to_string(i + 1) + " Score: ");

        auto go = std::make_unique<GameObject>();
        go->SetTag(TAG_SCOREBOARD);
        go->SetLocalPosition(WINDOW_W / 2.f - 100, 200.f + i * 60.f);
        go->AddComponent(std::make_unique<TextComponent>(
            *go, label + std::to_string(score), smallFont,
            i == 0 ? SDL_Color{ 255,255,0,255 } : SDL_Color{ 0,255,255,255 }
        ));
        m_pScene->Add(std::move(go));
    }

    auto hintGO = std::make_unique<GameObject>();
    hintGO->SetTag(TAG_SCOREBOARD);
    hintGO->SetLocalPosition(WINDOW_W / 2.f - 160, 450);
    hintGO->AddComponent(std::make_unique<TextComponent>(
        *hintGO, "Press Enter / A to play again", smallFont,
        SDL_Color{ 255, 255, 255, 255 }
    ));
    m_pScene->Add(std::move(hintGO));


    auto& input = InputManager::GetInstance();
    input.BindCommand(SDL_SCANCODE_RETURN,
        std::make_unique<RestartCommand>(this, m_GameMode),
        InputTriggerType::OnPressed);
    input.BindCommand(ControllerButton::A,
        std::make_unique<RestartCommand>(this, m_GameMode),
        InputTriggerType::OnPressed);
}