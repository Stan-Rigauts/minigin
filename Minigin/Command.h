#pragma once
#include "GameManagerComponent.h"
#include "GameState.h"

namespace dae
{
    class Command
    {
    public:
        virtual ~Command() = default;
        virtual void Execute() = 0;
    };

    class GameObject;
    class ScoreComponent;
    class HealthComponent;
    

   
    class MoveCommand final : public Command
    {
    public:
        MoveCommand(GameObject* player, float x, float y)
            : m_Player(player)
            , m_DirX(x)
            , m_DirY(y)
        {
        }

        void Execute() override;

    private:
        GameObject* m_Player{};
        float m_DirX{};
        float m_DirY{};
    };


    class ScoreCommand final : public Command
    {
    public:
        ScoreCommand(ScoreComponent* scoreComp, int amount)
            : m_pScore(scoreComp)
            , m_Amount(amount)
        {
        }

        void Execute() override;

    private:
        ScoreComponent* m_pScore{};
        int m_Amount{};
    };

   
    class HealthCommand final : public Command
    {
    public:
        HealthCommand(HealthComponent* healthComp, int damage)
            : m_pHealth(healthComp)
            , m_Damage(damage)
        {
        }

        void Execute() override;

    private:
        HealthComponent* m_pHealth{};
        int m_Damage{};
    };

    class RestartCommand final : public Command
    {
    public:
        RestartCommand(GameManagerComponent* manager, GameMode mode)
            : m_pManager(manager), m_Mode(mode) {
        }

        void Execute() override
        {
            if (m_pManager->GetGameState() == GameState::ScoreBoard)
                m_pManager->StartGame(m_Mode);
        }

    private:
        GameManagerComponent* m_pManager;
        GameMode              m_Mode;
    };
}