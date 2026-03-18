#pragma once
#include "Observer.h"
#include "Component.h"
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

        void OnNotify(GameEvent event, int value) override
        {
#if USE_STEAMWORKS
            if (event == GameEvent::ScoreChanged && value >= 500)
                UnlockAchievement("ACH_WIN_ONE_GAME");
#else
            (void)event; (void)value;
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