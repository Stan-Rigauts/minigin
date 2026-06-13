#include "GameManagerComponent.h"
#include "Command.h" 
#include "GameObject.h"
#include "GridComponent.h"
#include "HealthComponent.h"
#include "HighScoreManager.h"
#include "InputManager.h"
#include "LivesDisplayComponent.h"
#include "MoveComponent.h"
#include "PelletComponent.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "ScoreComponent.h"
#include "ScoreDisplayComponent.h"
#include "servicelocator.h"
#include "SoundIds.h"
#include "SpriteAnimatorComponent.h"
#include "TextComponent.h"
#include <string>


dae::GameManagerComponent::GameManagerComponent(GameObject& owner)
    : Component(owner)
{
	auto& input = InputManager::GetInstance();
    
    input.BindCommand(SDL_SCANCODE_F2,std::make_unique<MuteCommand>(),InputTriggerType::OnPressed);

    auto& highScoreManager = HighScoreManager::GetInstance();
    highScoreManager.Load();
    m_CurrentHighScore = highScoreManager.GetEntries().empty() ? 0 : highScoreManager.GetEntries().front().score;

}


void dae::GameManagerComponent::SetScene(Scene* scene)
{
    m_pScene = scene;
}

void dae::GameManagerComponent::SetLevels(const std::vector<std::string>& levelFiles , const std::vector<std::string>& levelTextures)
{
    m_LevelFiles = levelFiles;
    m_LevelTextures = levelTextures;
}

void dae::GameManagerComponent::BuildStartScreen()
{
    if (!m_pScene) return;
    m_GameState = GameState::Menu;
    
    auto font = ResourceManager::GetInstance().LoadFont("FontTitle.otf", 36);
    auto smallFont = ResourceManager::GetInstance().LoadFont("FontTitle.otf", 24);


    auto titleGO = std::make_unique<GameObject>();
    titleGO->SetTag(TAG_MENU);
    titleGO->SetLocalPosition(WINDOW_W / 2.f - 120, 80);
    titleGO->AddComponent(std::make_unique<TextComponent>(
        *titleGO, "MS.PACMAN", font, SDL_Color{ 255, 255, 0, 255 }));
    m_pScene->Add(std::move(titleGO));

    
    struct Option 
    { 
        const char* label;
        SDL_Scancode key; 
        ControllerButton btn; 
        GameMode mode; 
        float y; 
    };


    const Option options[] = {
        { "1 / A - Single Player", SDL_SCANCODE_1, ControllerButton::A, GameMode::Solo,   220.f },
        { "2 / B - Co-op",         SDL_SCANCODE_2, ControllerButton::B, GameMode::Duo,    290.f },
        { "3 / X - Versus",        SDL_SCANCODE_3, ControllerButton::X, GameMode::Versus, 360.f },
    };

    auto& input = InputManager::GetInstance();
    for (const auto& option : options)
    {
        auto go = std::make_unique<GameObject>();
        go->SetTag(TAG_MENU);
        go->SetLocalPosition(WINDOW_W / 2.f - 120, option.y);
        go->AddComponent(std::make_unique<TextComponent>(*go, option.label, smallFont, SDL_Color{ 255, 255, 255, 255 }));
        m_pScene->Add(std::move(go));

        input.BindCommand(option.key, std::make_unique<StartGameCommand>(this, option.mode, false), InputTriggerType::OnPressed);

        input.BindCommand(option.btn, std::make_unique<StartGameCommand>(this, option.mode, true), InputTriggerType::OnPressed);
        input.BindCommand(option.btn, std::make_unique<StartGameCommand>(this, option.mode, true), InputTriggerType::OnPressed, 1);
    }
}

void dae::GameManagerComponent::ClearMenu()
{
    if (m_pScene)
        m_pScene->RemoveObjectsWithTag(TAG_MENU);
}

void dae::GameManagerComponent::StartGame(GameMode mode, bool p1UsesController)
{
    m_FinalScores = { 0, 0 };
    m_P1UsesController = p1UsesController;
    m_GameMode = mode;
    m_GameState = GameState::Playing;
    m_CurrentLevel = 0;


    ClearPlayers();
    ClearLevelObjects();
    m_pScene->RemoveObjectsWithTag(TAG_SCOREBOARD);

    LoadLevel(0);

    auto& input = InputManager::GetInstance();

    input.BindCommand(SDL_SCANCODE_F1, std::make_unique<SkipLevelCommand>(this), InputTriggerType::OnPressed);

    
}
    
void dae::GameManagerComponent::Update(float deltaTime)
{
    if (m_Players.empty()) return;

    if (m_PowerPelletTimer > 0.f)
    {
        m_PowerPelletTimer -= deltaTime;
        if (m_PowerPelletTimer <= 0.f)
            for (auto* ghost : m_Ghosts)
                ghost->SetPowerPelletActive(false);
    }

    if (m_DeathPauseTimer > 0.f)
    {
        m_DeathPauseTimer -= deltaTime;
        if (m_DeathPauseTimer <= 0.f)
            SetGameFrozen(false);
        return;
    }

    if (m_PendingDeath.has_value())
    {
        int idx = m_PendingDeath->playerIndex;
        m_PendingDeath.reset();
        PlayerDied(idx);  
        return;
    }

    auto pacPos = m_Players[0].go->GetLocalPosition();
    for (auto* ghost : m_Ghosts)
        ghost->SetPacManPosition({ pacPos.x, pacPos.y });

    CheckGhostPacManCollision();


    
    int currentHighScore = 0;
    if (!m_Players.empty())
    {
        for (const auto& player : m_Players)
        {
            if (currentHighScore < player.score->GetScore())
                currentHighScore = player.score->GetScore();
        }
    }
    
    
     for (auto score : m_FinalScores)
     {
         if (currentHighScore < score)
             currentHighScore = score;
     }
    
     if (currentHighScore > m_CurrentHighScore)
     {
         m_CurrentHighScore = currentHighScore;
         if (m_pHighScoreDisplay)
            m_pHighScoreDisplay->SetStaticValue(m_CurrentHighScore);
     }
    
}


void dae::GameManagerComponent::PelletCollected(int playerIndex)
{
    servicelocator::get_sound_system().play(SND_PELLET, 1.0f);

    if (playerIndex >= 0 && playerIndex < static_cast<int>(m_Players.size()))
        m_Players[playerIndex].score->AddScore(10);

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
        m_FinalScores[playerIndex] = m_Players[playerIndex].score->GetScore();

        m_pScene->Remove(*m_Players[playerIndex].go);

        m_Players.erase(m_Players.begin() + playerIndex);

        bool anyAlive = false;

        for (auto& players : m_Players)
        {
            if (players.health->GetLives() > 0)
            {
                anyAlive = true;
                break;
            }
        }

        if (!anyAlive)
            EnterScoreBoard();
    }
    else
    {
       ResetPlayers(m_CurrentMapInfo, m_pGrid); 

       for (auto* ghost : m_Ghosts)
           ghost->Reset();
    }
}


void dae::GameManagerComponent::EnterScoreBoard()
{
    m_GameState = GameState::ScoreBoard;
    ClearLevelObjects();
    ClearPlayers();
    m_pScene->RemoveObjectsWithTag(TAG_HUD);
    m_pHighScoreDisplay = nullptr;
    int topScore = 0;

    for (int score : m_FinalScores)
        topScore = std::max(topScore, score);

    for (auto& player : m_Players)
        topScore = std::max(topScore, player.score->GetScore());

    auto& highScoreManager = HighScoreManager::GetInstance();
    highScoreManager.Load();

    if (highScoreManager.IsHighScore(topScore))
        BuildNameEntry(topScore);
    else
        BuildScoreBoard();
}

void dae::GameManagerComponent::BuildNameEntry(int score)
{
    if (!m_pScene) return;

    auto font = ResourceManager::GetInstance().LoadFont("FontTitle.otf", 42);
    auto smallFont = ResourceManager::GetInstance().LoadFont("FontTitle.otf", 28);

    // Title
    auto titleGO = std::make_unique<GameObject>();
    titleGO->SetTag(TAG_SCOREBOARD);
    titleGO->SetLocalPosition(WINDOW_W / 2.f - 160, 60);
    titleGO->AddComponent(std::make_unique<TextComponent>(
        *titleGO, "NEW HIGH SCORE!", font, SDL_Color{ 255, 255, 0, 255 }));
    m_pScene->Add(std::move(titleGO));

    // Score
    auto scoreGO = std::make_unique<GameObject>();
    scoreGO->SetTag(TAG_SCOREBOARD);
    scoreGO->SetLocalPosition(WINDOW_W / 2.f - 80, 140);
    scoreGO->AddComponent(std::make_unique<TextComponent>(
        *scoreGO, "Score: " + std::to_string(score), smallFont,
        SDL_Color{ 255, 255, 255, 255 }));
    m_pScene->Add(std::move(scoreGO));

    const char* hints[] = {
    "Left/Right: change letter",
    "A / Enter: confirm",
    "B / Back: delete"
    };

    for (int i = 0; i < 3; ++i)
    {
        auto hintGO = std::make_unique<GameObject>();
        hintGO->SetTag(TAG_SCOREBOARD);
        hintGO->SetLocalPosition(WINDOW_W / 2.f - 140, 200.f + i * 36.f);
        hintGO->AddComponent(std::make_unique<TextComponent>(
            *hintGO, hints[i], smallFont, SDL_Color{ 255, 0, 0, 255 }));
        m_pScene->Add(std::move(hintGO));
    }

    // Name display
    auto nameGO = std::make_unique<GameObject>();
    nameGO->SetTag(TAG_SCOREBOARD);
    nameGO->SetLocalPosition(WINDOW_W / 2.f - 60, 340.f);

    auto nameText = std::make_unique<TextComponent>(
        *nameGO, "AAA", font, SDL_Color{ 0, 255, 255, 255 });
    auto* nameTextPtr = nameText.get();
    nameGO->AddComponent(std::move(nameText));

    auto nameEntry = std::make_unique<NameEntryComponent>(*nameGO, score, nameTextPtr,
        [this](const std::string& name, int s)
        {
            HighScoreManager::GetInstance().Submit(name, s);
            m_pScene->RemoveObjectsWithTag(TAG_SCOREBOARD);
            BuildScoreBoard();
        });

    auto* nameEntryPtr = nameEntry.get();
    nameGO->AddComponent(std::move(nameEntry));
    m_pScene->Add(std::move(nameGO));

    // Input bindings
    auto& input = InputManager::GetInstance();
    input.BindCommand(SDL_SCANCODE_LEFT,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Left),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_RIGHT,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Right),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_RETURN,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Confirm),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_BACKSPACE,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Back),
        InputTriggerType::OnPressed);
    input.BindCommand(ControllerButton::DPadLeft,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Left),
        InputTriggerType::OnPressed);
    input.BindCommand(ControllerButton::DPadRight,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Right),
        InputTriggerType::OnPressed);
    input.BindCommand(ControllerButton::A,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Confirm),
        InputTriggerType::OnPressed);
    input.BindCommand(ControllerButton::B,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Back),
        InputTriggerType::OnPressed);

    input.BindCommand(ControllerButton::DPadLeft,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Left),
        InputTriggerType::OnPressed,1);
    input.BindCommand(ControllerButton::DPadRight,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Right),
        InputTriggerType::OnPressed,1);
    input.BindCommand(ControllerButton::A,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Confirm),
        InputTriggerType::OnPressed,1);
    input.BindCommand(ControllerButton::B,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Back),
        InputTriggerType::OnPressed,1);

    input.BindCommand(SDL_SCANCODE_LEFT,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Left),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_RIGHT,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Right),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_A,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Left),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_D,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Right),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_RETURN,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Confirm),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_BACKSPACE,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Back),
        InputTriggerType::OnPressed);
    input.BindCommand(SDL_SCANCODE_SPACE,
        std::make_unique<NameEntryCommand>(nameEntryPtr, NameEntryCommand::Action::Confirm),
        InputTriggerType::OnPressed);
}

void dae::GameManagerComponent::SetGameFrozen(bool frozen)
{
    for (auto* ghost : m_Ghosts)
        ghost->SetPaused(frozen);

    for (auto& player : m_Players)
        if (player.move)
            player.move->SetFrozen(frozen);
}

int dae::GameManagerComponent::CountPellets(const MapInfo& map) const
{
    int count = 0;
    for (int rows = 0; rows < map.rows; ++rows)
        for (int colums = 0; colums < map.cols; ++colums)
            if (map.grid[rows][colums] == TileType::Pellet ||
                map.grid[rows][colums] == TileType::PowerPellet)
                ++count;
    return count;
}

void dae::GameManagerComponent::PowerPelletCollected(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < static_cast<int>(m_Players.size()))
        m_Players[playerIndex].score->AddScore(50);
    m_PowerPelletTimer = POWER_PELLET_DURATION;
    m_GhostsEatenThisPower = 0;
    --m_RemainingPellets;
    for (auto* ghost : m_Ghosts)
        ghost->SetPowerPelletActive(true);

    if (m_RemainingPellets <= 0)
        LoadNextLevel();
}

void dae::GameManagerComponent::CheckGhostPacManCollision()
{
    if (m_Players.empty()) return;

    constexpr float EAT_RADIUS = 20.f;
    constexpr float DEATH_RADIUS = 18.f;
    constexpr float PAC_HALF = 24.f * 0.5f;

    for (int pi = 0; pi < static_cast<int>(m_Players.size()); ++pi)
    {
        auto& player = m_Players[pi];
        auto pacPos = player.go->GetLocalPosition();
        glm::vec2 pacCenter{ pacPos.x + PAC_HALF, pacPos.y + PAC_HALF };

        for (auto* ghost : m_Ghosts)
        {
            if (ghost->IsEatenByPacMan()) continue;

            glm::vec2 ghostCenter = ghost->GetPosition(); 

            float dist = glm::length(ghostCenter - pacCenter);

            if (ghost->IsPowerPelletActive())
            {
                if (dist > EAT_RADIUS) continue;
                ghost->SetEatenByPacMan(true);
                const int ghostScore = 200 * (1 << m_GhostsEatenThisPower); 
                ++m_GhostsEatenThisPower;
                player.score->AddScore(ghostScore);
                servicelocator::get_sound_system().play(SND_GHOST_EATEN, 1.0f);
            }
            else
            {
                if (dist > DEATH_RADIUS) continue;
                m_DeathPauseTimer = DEATH_PAUSE_DURATION;
                SetGameFrozen(true);
                servicelocator::get_sound_system().play(SND_DEATH, 1.0f);
                player.health->Damage();
                m_PendingDeath = { pi };
                return;
            }
        }
    }
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
        m_PlayerWon = true;
        EnterScoreBoard();
        return;
    }

    ClearLevelObjects();
    m_CurrentLevel = index;
    servicelocator::get_sound_system().play(SND_LEVELSTART, 1.0f);

    auto mapInfo = LevelLoader::LoadMap(m_LevelFiles[index]);
    m_CurrentMapInfo = mapInfo;
    m_RemainingPellets = CountPellets(mapInfo);

    const int mazePixelW = mapInfo.cols * TILE_SIZE;
    const int mazePixelH = mapInfo.rows * TILE_SIZE;
    const int mazeOffsetX = (WINDOW_W - mazePixelW) / 2;
    const int mazeOffsetY = (WINDOW_H - mazePixelH) / 2;

    //Grid
    auto gridGO = std::make_unique<GameObject>();
    gridGO->SetTag(TAG_LEVEL);
    auto gridComp = std::make_unique<GridComponent>(*gridGO, mapInfo, TILE_SIZE, mazeOffsetX, mazeOffsetY);
    auto* gridPtr = gridComp.get();
    m_pGrid = gridPtr;
    gridGO->AddComponent(std::move(gridComp));

    m_pScene->Add(std::move(gridGO));

    //Maze
    auto mazeGO = std::make_unique<GameObject>();
    mazeGO->SetTag(TAG_LEVEL);
    mazeGO->SetLocalPosition(static_cast<float>(mazeOffsetX),
        static_cast<float>(mazeOffsetY));
    auto render = std::make_unique<RenderComponent>(*mazeGO);
    render->SetTexture(m_LevelTextures[index]);
    render->SetLayer(0); 
    mazeGO->AddComponent(std::move(render));
    m_pScene->Add(std::move(mazeGO));




    if (m_Players.empty())
        SpawnPlayers(mapInfo, gridPtr);
    else
    {
        ResetPlayers(mapInfo, gridPtr);
        for (auto& player : m_Players)
            player.move->FreezeFor(4.f);
    }

    SpawnGhosts(gridPtr);
    SpawnPellets(mapInfo);

  
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

    const int spawnRow = (mapInfo.rows / 2) + 2;
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
        r1->SetSize(24.f, 24.f);
        r1->SetLayer(3);
        p1->AddComponent(std::move(r1));

        auto pacAnim1 = std::make_unique<SpriteAnimatorComponent>(*p1, 16, 16);
        pacAnim1->SetBaseTexture("MSPacmanSprite.png");
        pacAnim1->SetRenderSize(24.f, 24.f);
        pacAnim1->AddAnimation({ "walk_right", 0, 2, 0, 0.1f, true });
        pacAnim1->AddAnimation({ "walk_left",  0, 2, 1, 0.1f, true });
        pacAnim1->AddAnimation({ "walk_up",    0, 2, 2, 0.1f, true });
        pacAnim1->AddAnimation({ "walk_down",  0, 2, 3, 0.1f, true });
        auto* pacAnimPtr1 = pacAnim1.get();
        p1->AddComponent(std::move(pacAnim1));

        auto moveComp1 = std::make_unique<MoveComponent>(*p1, 100.f);
        auto* movePtr1 = moveComp1.get();
        movePtr1->SetGrid(grid);
        movePtr1->SetGameManager(this);
        movePtr1->FreezeFor(4.f);
        movePtr1->SetAnimator(pacAnimPtr1);
        p1->AddComponent(std::move(moveComp1));

        auto hpComp1 = std::make_unique<HealthComponent>(*p1, 4);
        auto* hpPtr1 = hpComp1.get();
        p1->AddComponent(std::move(hpComp1));

        auto scComp1 = std::make_unique<ScoreComponent>(*p1, 0);
        auto* scPtr1 = scComp1.get();
        p1->AddComponent(std::move(scComp1));

        auto* p1Ptr = p1.get();
        m_pScene->Add(std::move(p1));

        for (auto& go : m_pScene->GetObjects())
            if (auto* pellet = go->GetComponent<PelletComponent>())
                pellet->AddPlayer(p1Ptr);

        if (m_P1UsesController)
        {
            input.BindCommand(ControllerButton::DPadUp, std::make_unique<MoveCommand>(p1Ptr, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadDown, std::make_unique<MoveCommand>(p1Ptr, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadLeft, std::make_unique<MoveCommand>(p1Ptr, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadRight, std::make_unique<MoveCommand>(p1Ptr, 1.f, 0.f), InputTriggerType::WhilePressed);
        }
        else
        {
            input.BindCommand(SDL_SCANCODE_W, std::make_unique<MoveCommand>(p1Ptr, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_S, std::make_unique<MoveCommand>(p1Ptr, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_A, std::make_unique<MoveCommand>(p1Ptr, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_D, std::make_unique<MoveCommand>(p1Ptr, 1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_UP, std::make_unique<MoveCommand>(p1Ptr, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_DOWN, std::make_unique<MoveCommand>(p1Ptr, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_LEFT, std::make_unique<MoveCommand>(p1Ptr, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_RIGHT, std::make_unique<MoveCommand>(p1Ptr, 1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadUp, std::make_unique<MoveCommand>(p1Ptr, 0.f, -1.f), InputTriggerType::WhilePressed, 1);
            input.BindCommand(ControllerButton::DPadDown, std::make_unique<MoveCommand>(p1Ptr, 0.f, 1.f), InputTriggerType::WhilePressed, 1);
            input.BindCommand(ControllerButton::DPadLeft, std::make_unique<MoveCommand>(p1Ptr, -1.f, 0.f), InputTriggerType::WhilePressed, 1);
            input.BindCommand(ControllerButton::DPadRight, std::make_unique<MoveCommand>(p1Ptr, 1.f, 0.f), InputTriggerType::WhilePressed, 1);
           
        }

       
        {
            auto headerGO = std::make_unique<GameObject>();
            headerGO->SetTag(TAG_HUD);
            headerGO->SetLocalPosition((WINDOW_W - 400.f) / 2.f, 0.f);

            auto headerRender = std::make_unique<RenderComponent>(*headerGO);
            headerRender->SetTexture("Score.png");

            if (m_GameMode == GameMode::Solo || m_GameMode == GameMode::Versus)
            {
                headerRender->SetSourceRect(0, 0, 300, 32);
                headerRender->SetSize(300, 32.f);
            }
            else
            {
                headerRender->SetSourceRect(0, 0, 400, 32);
                headerRender->SetSize(400.f, 32.f);
            }

            headerRender->SetLayer(3);
            headerGO->AddComponent(std::move(headerRender));
            m_pScene->Add(std::move(headerGO));
        }

        const float SCORE_LEFT = (WINDOW_W - 400.f) / 2.f;

        auto p1ScoreGO = std::make_unique<GameObject>();
        p1ScoreGO->SetTag(TAG_HUD);
        p1ScoreGO->AddComponent(std::make_unique<ScoreDisplayComponent>(
            *p1ScoreGO, *scPtr1, m_pScene, TAG_HUD,
            SCORE_LEFT, 16.f, 16.f));
        m_pScene->Add(std::move(p1ScoreGO));

        const float ICON_SIZE = 30.f;
        const float ICONS_Y = static_cast<float>(WINDOW_H) - ICON_SIZE - 8.f;

        auto p1LivesGO = std::make_unique<GameObject>();
        p1LivesGO->SetTag(TAG_HUD);
        p1LivesGO->AddComponent(std::make_unique<LivesDisplayComponent>(
            *p1LivesGO, *hpPtr1, m_pScene, "Lives.png",
            ICON_SIZE, 16.f, ICONS_Y));
        m_pScene->Add(std::move(p1LivesGO));

        m_Players.push_back({ p1Ptr, hpPtr1, scPtr1, movePtr1, spawnRow, spawnCol - 1 });

        auto& highScore = HighScoreManager::GetInstance();
        highScore.Load();

        int best = highScore.GetEntries().empty() ? 0 : highScore.GetEntries().front().score;

        auto highScoreGO = std::make_unique<GameObject>();
        highScoreGO->SetTag(TAG_HUD);

        auto highScoreDisplay = std::make_unique<ScoreDisplayComponent>(
            *highScoreGO,
            best,
            m_pScene,
            TAG_HUD,
            SCORE_LEFT + 160.f,
            16.f,
            16.f
        );

        m_pHighScoreDisplay = highScoreDisplay.get();

        highScoreGO->AddComponent(std::move(highScoreDisplay));
        m_pScene->Add(std::move(highScoreGO));
    }

    // Player 2 (Duo only)
    if (m_GameMode == GameMode::Duo)
    {
        const float x = static_cast<float>(mazeOffsetX + (spawnCol + 1) * TILE_SIZE);
        const float y = static_cast<float>(mazeOffsetY + spawnRow * TILE_SIZE);

        auto p2 = std::make_unique<GameObject>();
        p2->SetTag(TAG_PLAYER);
        p2->SetLocalPosition(x, y);

        auto r2 = std::make_unique<RenderComponent>(*p2);
        r2->SetTexture("MSPacmanSprite.png");
        r2->SetSourceRect(0, 0, 16, 16);
        r2->SetSize(24.f, 24.f);
        r2->SetLayer(3);
        p2->AddComponent(std::move(r2));

        auto pacAnim2 = std::make_unique<SpriteAnimatorComponent>(*p2, 16, 16);
        pacAnim2->SetBaseTexture("MSPacmanSprite.png");
        pacAnim2->SetRenderSize(24.f, 24.f);
        pacAnim2->AddAnimation({ "walk_right", 0, 2, 0, 0.1f, true });
        pacAnim2->AddAnimation({ "walk_left",  0, 2, 1, 0.1f, true });
        pacAnim2->AddAnimation({ "walk_up",    0, 2, 2, 0.1f, true });
        pacAnim2->AddAnimation({ "walk_down",  0, 2, 3, 0.1f, true });
        auto* pacAnimPtr2 = pacAnim2.get();
        p2->AddComponent(std::move(pacAnim2));

        auto moveComp2 = std::make_unique<MoveComponent>(*p2, 100.f);
        auto* movePtr2 = moveComp2.get();
        movePtr2->SetGrid(grid);
        movePtr2->SetGameManager(this);
        movePtr2->SetAnimator(pacAnimPtr2);
        movePtr2->FreezeFor(4.f);
        p2->AddComponent(std::move(moveComp2));


        auto hpComp2 = std::make_unique<HealthComponent>(*p2, 4);
        auto* hpPtr2 = hpComp2.get();
        p2->AddComponent(std::move(hpComp2));

        auto scComp2 = std::make_unique<ScoreComponent>(*p2, 0);
        auto* scPtr2 = scComp2.get();
        p2->AddComponent(std::move(scComp2));

        auto* p2Ptr = p2.get();
        m_pScene->Add(std::move(p2));


        for (auto& go : m_pScene->GetObjects())
            if (auto* pellet = go->GetComponent<PelletComponent>())
                pellet->AddPlayer(p2Ptr);

        if (m_P1UsesController)
        {
            input.BindCommand(SDL_SCANCODE_W, std::make_unique<MoveCommand>(p2Ptr, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_S, std::make_unique<MoveCommand>(p2Ptr, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_A, std::make_unique<MoveCommand>(p2Ptr, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_D, std::make_unique<MoveCommand>(p2Ptr, 1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_UP, std::make_unique<MoveCommand>(p2Ptr, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_DOWN, std::make_unique<MoveCommand>(p2Ptr, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_LEFT, std::make_unique<MoveCommand>(p2Ptr, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_RIGHT, std::make_unique<MoveCommand>(p2Ptr, 1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadUp, std::make_unique<MoveCommand>(p2Ptr, 0.f, -1.f), InputTriggerType::WhilePressed,1);
            input.BindCommand(ControllerButton::DPadDown, std::make_unique<MoveCommand>(p2Ptr, 0.f, 1.f), InputTriggerType::WhilePressed,1);
            input.BindCommand(ControllerButton::DPadLeft, std::make_unique<MoveCommand>(p2Ptr, -1.f, 0.f), InputTriggerType::WhilePressed,1);
            input.BindCommand(ControllerButton::DPadRight, std::make_unique<MoveCommand>(p2Ptr, 1.f, 0.f), InputTriggerType::WhilePressed,1);

        }
        else
        {
            input.BindCommand(ControllerButton::DPadUp, std::make_unique<MoveCommand>(p2Ptr, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadDown, std::make_unique<MoveCommand>(p2Ptr, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadLeft, std::make_unique<MoveCommand>(p2Ptr, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadRight, std::make_unique<MoveCommand>(p2Ptr, 1.f, 0.f), InputTriggerType::WhilePressed);
          
        }

        // HUD
        const float SCORE_LEFT = (WINDOW_W - 400.f) / 2.f;
        auto p2ScoreGO = std::make_unique<GameObject>();
        p2ScoreGO->SetTag(TAG_HUD);
        p2ScoreGO->AddComponent(std::make_unique<ScoreDisplayComponent>(
            *p2ScoreGO, *scPtr2, m_pScene, TAG_HUD,
            SCORE_LEFT + 304.f, 16.f, 16.f));
        m_pScene->Add(std::move(p2ScoreGO));
       
        const float ICON_SIZE = 30.f;
        const float ICONS_Y = static_cast<float>(WINDOW_H) - ICON_SIZE - 8.f;
        auto p2LivesGO = std::make_unique<GameObject>();
        p2LivesGO->SetTag(TAG_HUD);
        p2LivesGO->AddComponent(std::make_unique<LivesDisplayComponent>(
            *p2LivesGO, *hpPtr2, m_pScene, "Lives.png",
            ICON_SIZE, static_cast<float>(WINDOW_W) - (4 * (ICON_SIZE + 4.f)) - 16.f, ICONS_Y));
        m_pScene->Add(std::move(p2LivesGO));
        m_Players.push_back({ p2Ptr, hpPtr2, scPtr2, movePtr2, spawnRow, spawnCol + 1 });
    }
}

void dae::GameManagerComponent::ResetPlayers(const MapInfo& mapInfo, GridComponent* grid)
{
    const int mazeOffsetX = (WINDOW_W - mapInfo.cols * TILE_SIZE) / 2;
    const int mazeOffsetY = (WINDOW_H - mapInfo.rows * TILE_SIZE) / 2;

    for (auto& player : m_Players)
    {
        const float x = static_cast<float>(mazeOffsetX + player.spawnCol * TILE_SIZE);
        const float y = static_cast<float>(mazeOffsetY + player.spawnRow * TILE_SIZE);
        player.go->SetLocalPosition(x, y);
        player.move->SetGrid(grid);
    }
}

void dae::GameManagerComponent::SpawnPellets(const MapInfo& mapInfo)
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
			render->SetLayer(1);
            pelletGO->AddComponent(std::move(render));

            pelletGO->AddComponent(std::make_unique<PelletComponent>(
                *pelletGO, this, playerGOs, isPower, 9.f));

            m_pScene->Add(std::move(pelletGO));
        }
    }
}


void dae::GameManagerComponent::BuildScoreBoard()
{
    if (!m_pScene) return;
    

    auto font = ResourceManager::GetInstance().LoadFont("FontTitle.otf", 70);
    auto smallFont = ResourceManager::GetInstance().LoadFont("FontTitle.otf", 24);

    auto titleGO = std::make_unique<GameObject>();
    titleGO->SetTag(TAG_SCOREBOARD);
    titleGO->SetLocalPosition(WINDOW_W / 2.f - 160, 60);
    titleGO->AddComponent(std::make_unique<TextComponent>(
        *titleGO,
        m_PlayerWon ? "YOU WIN!" : "GAME OVER",
        font,
        m_PlayerWon ? SDL_Color{ 0, 255, 0, 255 } : SDL_Color{ 255, 0, 0, 255 }));
    m_pScene->Add(std::move(titleGO));

    int index = 0;

    for (const auto& player : m_Players)
    {
        const int score = player.score->GetScore();

        std::string label =
            (m_GameMode == GameMode::Versus && index == 1)
            ? "Ghost Score: "
            : "P" + std::to_string(index + 1) + " Score: ";

        auto go = std::make_unique<GameObject>();
        go->SetTag(TAG_SCOREBOARD);
        go->SetLocalPosition(WINDOW_W / 2.f - 100, 140.f + index * 50.f);

        go->AddComponent(std::make_unique<TextComponent>(
            *go,
            label + std::to_string(score),
            smallFont,
            index == 0
            ? SDL_Color{ 255,255,0,255 }
            : SDL_Color{ 0,255,255,255 }
        ));

        m_pScene->Add(std::move(go));

        ++index;
    }

    // High score list
    auto hsTitle = std::make_unique<GameObject>();
    hsTitle->SetTag(TAG_SCOREBOARD);
    hsTitle->SetLocalPosition(WINDOW_W / 2.f - 80, 260);
    hsTitle->AddComponent(std::make_unique<TextComponent>(
        *hsTitle, "HIGH SCORES", smallFont, SDL_Color{ 255, 165, 0, 255 }));
    m_pScene->Add(std::move(hsTitle));

    const auto& entries = HighScoreManager::GetInstance().GetEntries();
    index = 0;
    for (const auto& entry : entries)
    {
        std::string line = std::to_string(index + 1) + ". " +
            entry.name + "   " +
            std::to_string(entry.score);

        auto go = std::make_unique<GameObject>();
        go->SetTag(TAG_SCOREBOARD);
        go->SetLocalPosition(WINDOW_W / 2.f - 80, 300.f + index * 36.f);
        go->AddComponent(std::make_unique<TextComponent>(
            *go, line, smallFont, SDL_Color{ 255, 255, 255, 255 }));

        m_pScene->Add(std::move(go));

        ++index;
    }

    
}

void dae::GameManagerComponent::SpawnGhosts(GridComponent* grid)
{
    const float MAP_W =
        static_cast<float>(grid->GetCols() * grid->GetTileSize() + grid->GetOffsetX());

    const float MAP_H =
        static_cast<float>(grid->GetRows() * grid->GetTileSize() + grid->GetOffsetY());

    auto TileToWorld = [&](float row, float col)
        {
            const float ts = static_cast<float>(grid->GetTileSize());
            float x = static_cast<float>(grid->GetOffsetX()) + col * ts + ts * 0.5f;
            float y = static_cast<float>(grid->GetOffsetY()) + row * ts + ts * 0.5f;
            return glm::vec2{ x, y };
        };

    struct GhostDesc
    {
        Ghost::Type type;
        const char* chaseSprite;
        const char* frightenedSprite;
        const char* eyesSprite;
        float row;
        float col;
        glm::vec2 scatterCorner;
        float frozenTime = 0.f;
    };

    const float centerRow = (static_cast<float>(grid->GetRows()) - 1.f) * 0.5f;
    const float centerCol = (static_cast<float>(grid->GetCols()) - 1.f) * 0.5f;

    glm::vec2 doorWorldPos{};
    const float ts = static_cast<float>(grid->GetTileSize());
    for (int row = 0; row < grid->GetRows(); ++row)
        for (int col = 0; col < grid->GetCols(); ++col)
            if (grid->GetTile(col, row) == dae::TileType::GhostDoor)
            {
                doorWorldPos = {
                    static_cast<float>(grid->GetOffsetX()) + (col - 0.5f) * ts + ts * 0.5f,
                    static_cast<float>(grid->GetOffsetY()) + (row - 1) * ts + ts * 0.5f
                };
            }

    glm::vec2 houseCenter = {
        static_cast<float>(grid->GetOffsetX()) + centerCol * ts + ts * 0.5f,
        static_cast<float>(grid->GetOffsetY()) + (centerRow - 1.f) * ts + ts * 0.5f
    };

    const GhostDesc descs[] =
    {
        // Blinky
        {
            Ghost::Type::Blinky,
            "RedGhost.png", "FreightenedGhost.png", "DeadEyesGhosts.png",
            centerRow - 4.f, centerCol - 0.5f,
            { MAP_W - 16.f, 16.f },
            4.f
        },
        // Inky
        {
            Ghost::Type::Inky,
            "CyanGhost.png", "FreightenedGhost.png", "DeadEyesGhosts.png",
            centerRow - 1.f, centerCol - 2.f,
            { MAP_W - 16.f, MAP_H - 16.f },
            9.f
        },
        // Pinky
        {
            Ghost::Type::Pinky,
            "PinkGhost.png", "FreightenedGhost.png", "DeadEyesGhosts.png",
            centerRow - 1.f, centerCol,
            { 16.f, 16.f },
            5.f
        },
        // Clyde
        {
            Ghost::Type::Clyde,
            "OrangeGhost.png", "FreightenedGhost.png", "DeadEyesGhosts.png",
            centerRow - 1.f, centerCol + 2.f,
            { 16.f, MAP_H - 16.f },
            14.f
        },
    };

    m_Ghosts.clear();
    m_PlayerControlledGhost = nullptr;

    for (const auto& desc : descs)
    {
        glm::vec2 worldPos = TileToWorld(desc.row, desc.col);
        auto go = std::make_unique<dae::GameObject>();
        auto ghostComponent = std::make_unique<Ghost>(*go);

        Ghost* ghost = ghostComponent.get();
        ghost->SetDoorWorldPosition(doorWorldPos);
        ghost->SetExitPath({ houseCenter, doorWorldPos });
        ghost->SetType(desc.type);
        ghost->SetGrid(grid);
        ghost->SetStartsOutside(desc.type == Ghost::Type::Blinky);
        ghost->SetFrozen(true, desc.frozenTime);
        ghost->SetInitialFrozenTime(desc.frozenTime);
        ghost->SetBasePosition(TileToWorld(descs[0].row, descs[0].col));
        ghost->SetStartPosition(worldPos);
        ghost->SetScatterTarget(desc.scatterCorner);
        ghost->SetChaseSprite(desc.chaseSprite);
        ghost->SetFrightenedSprite(desc.frightenedSprite);
        ghost->SetEyesSprite(desc.eyesSprite);
        ghost->SetWhiteSprite("WhiteGhost.png");

        go->SetLocalPosition(worldPos.x, worldPos.y);
        m_Ghosts.push_back(ghost);

        go->AddComponent(std::move(ghostComponent));
        go->SetTag(TAG_LEVEL);

        auto render = std::make_unique<dae::RenderComponent>(*go);
        render->SetTexture(desc.chaseSprite);
        render->SetSourceRect(0, 0, 16, 16);
        render->SetSize(28.f, 28.f);
        render->SetLayer(2);
        go->AddComponent(std::move(render));

        auto animator = std::make_unique<dae::SpriteAnimatorComponent>(*go, 16, 16);
        animator->SetBaseTexture(desc.chaseSprite);
        animator->SetRenderSize(28.f, 28.f);

        dae::Animation walkRight{ "walk_right", 0, 1, 0, 0.15f, true };
        dae::Animation walkLeft{ "walk_left",  2, 3, 0, 0.15f, true };
        dae::Animation walkUp{ "walk_up",    4, 5, 0, 0.15f, true };
        dae::Animation walkDown{ "walk_down",  6, 7, 0, 0.15f, true };

        dae::Animation frightened{ "frightened", 0, 1, 0, 0.15f, true };

        dae::Animation white{ "white", 0, 1, 0, 0.15f, true };

        dae::Animation eyesRight{ "eyes_right", 0, 0, 0, 0.15f, false };
        dae::Animation eyesLeft{ "eyes_left",  1, 1, 0, 0.15f, false };
        dae::Animation eyesUp{ "eyes_up",    2, 2, 0, 0.15f, false };
        dae::Animation eyesDown{ "eyes_down",  3, 3, 0, 0.15f, false };

        animator->AddAnimation(walkRight);
        animator->AddAnimation(walkLeft);
        animator->AddAnimation(walkUp);
        animator->AddAnimation(walkDown);
        animator->AddAnimation(frightened);
        animator->AddAnimation(white);
        animator->AddAnimation(eyesRight);
        animator->AddAnimation(eyesLeft);
        animator->AddAnimation(eyesUp);
        animator->AddAnimation(eyesDown);

        go->AddComponent(std::move(animator));

        auto* goPtr = go.get();

        m_pScene->Add(std::move(go));
        ghost->Start();

        if (auto* anim = goPtr->GetComponent<dae::SpriteAnimatorComponent>())
            ghost->SetAnimator(anim);

        // Versus
        if (m_GameMode == GameMode::Versus && desc.type == Ghost::Type::Blinky)
        {
            ghost->SetPlayerControlled(true);
            ghost->SetFrozen(true, 4.f);
            m_PlayerControlledGhost = ghost;
        }
    }

    if (m_GameMode == GameMode::Versus && m_PlayerControlledGhost)
    {
        auto& input = InputManager::GetInstance();

        if (m_P1UsesController)
        {
            input.BindCommand(SDL_SCANCODE_W, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_S, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_A, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_D, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_UP, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_DOWN, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_LEFT, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(SDL_SCANCODE_RIGHT, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadUp, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 0.f, -1.f), InputTriggerType::WhilePressed,1);
            input.BindCommand(ControllerButton::DPadDown, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 0.f, 1.f), InputTriggerType::WhilePressed, 1);
            input.BindCommand(ControllerButton::DPadLeft, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, -1.f, 0.f), InputTriggerType::WhilePressed, 1);
            input.BindCommand(ControllerButton::DPadRight, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 1.f, 0.f), InputTriggerType::WhilePressed, 1);

        }
        else
        {
            input.BindCommand(ControllerButton::DPadUp, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 0.f, -1.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadDown, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 0.f, 1.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadLeft, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, -1.f, 0.f), InputTriggerType::WhilePressed);
            input.BindCommand(ControllerButton::DPadRight, std::make_unique<GhostMoveCommand>(m_PlayerControlledGhost, 1.f, 0.f), InputTriggerType::WhilePressed);
        }
    }
}