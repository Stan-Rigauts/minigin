#pragma once
#include "BaseCommand.h"
#include "GameState.h"
#include "GameManagerComponent.h"
#include "NameEntryComponent.h"
#include "serviceLocator.h"

namespace dae
{
    class GameObject;
    class ScoreComponent;
    class HealthComponent;
    class GameManagerComponent;

    class MoveCommand final : public Command
    {
    public:
        MoveCommand(GameObject* player, float x, float y)
            : m_Player(player), m_DirX(x), m_DirY(y) {
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
            : m_pScore(scoreComp), m_Amount(amount) {
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
            : m_pHealth(healthComp), m_Damage(damage) {
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
        void Execute() override;
    private:
        GameManagerComponent* m_pManager;
        GameMode m_Mode;
    };

    class GameManagerComponent;

    class StartGameCommand final : public Command
    {
    public:
        StartGameCommand(GameManagerComponent* gm, GameMode mode, bool p1UsesController = false);
        void Execute() override;
    private:
        GameManagerComponent* m_GM;
        GameMode m_Mode;
        bool m_P1UsesController;
    };

    class GameManagerComponent;

    class GoToMenuCommand final : public Command
    {
    public:
        GoToMenuCommand(GameManagerComponent* gm);
        void Execute() override;
    private:
        GameManagerComponent* m_GM;
    };

    class GhostMoveCommand final : public Command
    {
    public:
        GhostMoveCommand(Ghost* ghost, float x, float y)
            : m_Ghost(ghost), m_DirX(x), m_DirY(y) {
        }
        void Execute() override
        {
            if (m_Ghost && m_Ghost->IsPlayerControlled())
                m_Ghost->SetInputDirection(m_DirX, m_DirY);
        }
    private:
        Ghost* m_Ghost;
        float m_DirX;
        float m_DirY;
    };

    class SkipLevelCommand final : public Command
    {
    public:
        explicit SkipLevelCommand(dae::GameManagerComponent* gm)
            : m_pGM(gm) {
        }

        void Execute() override
        {
            if (m_pGM)
                m_pGM->LoadNextLevel();
        }

    private:
        dae::GameManagerComponent* m_pGM{};
    };


    class NameEntryCommand final : public Command
    {
    public:
        enum class Action
        {
            Left,
            Right,
            Confirm,
            Back
		};
        NameEntryCommand(NameEntryComponent* entry, Action action)
            : m_Entry(entry)
            , m_pActive(entry->GetActiveFlag())
            , m_Action(action) {
        }

        void Execute() override
        {
            if (!m_pActive || !*m_pActive) return; 
            switch (m_Action)
            {
            case Action::Left:    m_Entry->MoveLeft();   break;
            case Action::Right:   m_Entry->MoveRight();  break;
            case Action::Confirm: m_Entry->Confirm();    break;
            case Action::Back:    m_Entry->Back();       break;
            }
        }
    private:
        NameEntryComponent* m_Entry{};
        std::shared_ptr<bool> m_pActive;
        Action m_Action{};
    };


    class MuteCommand final : public Command
    {
    public:
        void Execute() override
        {
            auto& ss = servicelocator::get_sound_system();
            ss.SetMuted(!ss.IsMuted());
        }
    };

}