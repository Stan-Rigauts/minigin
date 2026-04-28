#pragma once
#include "Component.h"
#include "GameState.h"
#include <vector>
#include <string>

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

        void SetScene(Scene* scene);
        void SetLevels(const std::vector<std::string>& levelFiles,
            const std::vector<std::string>& levelTextures);

        void StartGame(GameMode mode);
        void PelletCollected();
        void PlayerDied(int playerIndex);

        GameMode  GetGameMode()  const { return m_GameMode; }
        GameState GetGameState() const { return m_GameState; }
        int       GetCurrentLevel() const { return m_CurrentLevel; }

        void EnterScoreBoard();
    private:
        void LoadLevel(int index);
        void LoadNextLevel();
        void ClearLevelObjects();
        void SpawnPlayers(const MapInfo& mapInfo, GridComponent* grid);
        void ResetPlayers(const MapInfo& mapInfo, GridComponent* grid);
        void SpawnPellets(const MapInfo& mapInfo, GridComponent* grid);
        void ClearPlayers();
        void BuildScoreBoard();
        int  CountPellets(const MapInfo& map) const;

        Scene* m_pScene = nullptr;

        std::vector<std::string> m_LevelFiles;
        std::vector<std::string> m_LevelTextures;

        GameState m_GameState = GameState::Playing;
        GameMode  m_GameMode = GameMode::Solo;
        int       m_CurrentLevel = 0;
        int       m_RemainingPellets = 0;

        struct PlayerEntry
        {
            GameObject* go = nullptr;
            HealthComponent* health = nullptr;
            ScoreComponent* score = nullptr;
            MoveComponent* move = nullptr;
            int spawnRow = 0;
            int spawnCol = 0;
        };
        std::vector<PlayerEntry> m_Players;

        static constexpr const char* TAG_LEVEL = "level_object";
        static constexpr const char* TAG_PLAYER = "player_object";
        static constexpr const char* TAG_HUD = "hud_object";
        static constexpr const char* TAG_SCOREBOARD = "scoreboard_object";
        static constexpr int TILE_SIZE = 16;
        static constexpr int WINDOW_W = 869;
        static constexpr int WINDOW_H = 640;
    };
}