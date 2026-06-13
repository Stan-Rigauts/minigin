#pragma once
#include "Component.h"
#include "GameState.h"
#include "Ghost.h"
#include "LevelLoader.h"
#include "ScoreDisplayComponent.h"
#include <array>
#include <string>
#include <vector>

namespace dae
{
    class Scene;
    class GridComponent;
    class HealthComponent;
    class ScoreComponent;
    class MoveComponent;
    class TextComponent;
    struct MapInfo;

    class GameManagerComponent final : public Component
    {
    public:
        explicit GameManagerComponent(GameObject& owner);

        void Update(float deltaTime);

        void SetScene(Scene* scene);
        void SetLevels(const std::vector<std::string>& levelFiles, const std::vector<std::string>& levelTextures);
        void SetGameFrozen(bool frozen);
        void SetPlayerControlledGhost(Ghost* ghost) { m_PlayerControlledGhost = ghost; }

        void BuildStartScreen();
        void ClearMenu();
        void StartGame(GameMode mode, bool p1UsesController = false);
        void LoadNextLevel();

        void PelletCollected(int playerIndex);
        void PowerPelletCollected(int playerIndex);
        void PlayerDied(int playerIndex);
        void EnterScoreBoard();
        void BuildNameEntry(int score);

        bool IsPlaying()       const { return m_GameState == GameState::Playing; }
        int GetCurrentLevel() const { return m_CurrentLevel; }
        GameMode GetGameMode()     const { return m_GameMode; }
        GameState GetGameState()    const { return m_GameState; }

    private:
        struct PlayerEntry
        {
            GameObject* go = nullptr;
            HealthComponent* health = nullptr;
            ScoreComponent* score = nullptr;
            MoveComponent* move = nullptr;
            int spawnRow = 0;
            int spawnCol = 0;
        };

        struct PendingDeath { int playerIndex; };

        void LoadLevel(int index);
        void ClearLevelObjects();
        void ClearPlayers();
        void SpawnPlayers(const MapInfo& mapInfo, GridComponent* grid);
        void ResetPlayers(const MapInfo& mapInfo, GridComponent* grid);
        void SpawnPellets(const MapInfo& mapInfo);
        void SpawnGhosts(GridComponent* grid);
        void BuildScoreBoard();
        void CheckGhostPacManCollision();
        int  CountPellets(const MapInfo& map) const;

        Scene* m_pScene = nullptr;
        GridComponent* m_pGrid = nullptr;
        Ghost* m_PlayerControlledGhost = nullptr;

        std::vector<Ghost*> m_Ghosts;
        std::vector<std::string> m_LevelFiles;
        std::vector<std::string> m_LevelTextures;
        std::vector<PlayerEntry> m_Players;
        std::array<int, 2> m_FinalScores{ 0, 0 };

        MapInfo m_CurrentMapInfo{};
        GameState m_GameState = GameState::Playing;
        GameMode m_GameMode = GameMode::Solo;

        ScoreDisplayComponent* m_pHighScoreDisplay = nullptr;

        bool m_PlayerWon{ false };
        int m_CurrentLevel = 0;
        int m_RemainingPellets = 0;
        int m_GhostsEatenThisPower = 0;
        int m_CurrentHighScore = 0;

        float m_DeathPauseTimer = 0.f;
        float m_PowerPelletTimer = 0.f;

        bool m_P1UsesController = false;

        std::optional<PendingDeath> m_PendingDeath{};

        static constexpr float DEATH_PAUSE_DURATION = 2.f;
        static constexpr float POWER_PELLET_DURATION = 8.f;
        static constexpr int TILE_SIZE = 16;
        static constexpr int WINDOW_W = 869;
        static constexpr int WINDOW_H = 640;
        static constexpr const char* TAG_LEVEL = "level_object";
        static constexpr const char* TAG_PLAYER = "player_object";
        static constexpr const char* TAG_HUD = "hud_object";
        static constexpr const char* TAG_MENU = "menu_object";
        static constexpr const char* TAG_SCOREBOARD = "scoreboard_object";
    };
}