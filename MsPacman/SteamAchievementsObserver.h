#pragma once
#include "Observer.h"
#include "Component.h"
#include "ScoreComponent.h"
#if USE_STEAMWORKS
#include <steam_api.h>
#endif

namespace dae
{
    class SteamAchievementsObserver final : public Component, public Observer
    {
    public:
        explicit SteamAchievementsObserver(GameObject& owner)
            : Component(owner)
        {
        }
        ~SteamAchievementsObserver() override = default;

        void OnNotify(GameEvent event, dae::GameObject* owner) override
        {
#if USE_STEAMWORKS
            if (event == GameEvent::ScoreChanged)
            {
                auto score = owner->GetComponent<ScoreComponent>()->GetScore();
                if (score >= 500)
                    UnlockAchievement("ACH_WIN_ONE_GAME");
            }
#else
            (void)event; (void)owner;
#endif
        }
    private:
#if USE_STEAMWORKS
        void UnlockAchievement(const char* id)
        {
            if (auto* stats = SteamUserStats())
            {
                stats->SetAchievement(id);
                stats->StoreStats();
            }
        }
#endif
    };
}