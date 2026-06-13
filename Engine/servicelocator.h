#pragma once
#include "sound_system.h"
#include "null_sound_system.h"
#include <memory>

class servicelocator final
{
public:
    static sound_system& get_sound_system() { return *soundSystemInstance; }

    static void register_sound_system(std::unique_ptr<sound_system>&& soundsystem)
    {
        soundSystemInstance = soundsystem == nullptr
            ? std::make_unique<null_sound_system>()
            : std::move(soundsystem);
    }

private:
    static std::unique_ptr<sound_system> soundSystemInstance;
};