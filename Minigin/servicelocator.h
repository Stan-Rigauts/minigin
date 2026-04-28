#pragma once
#include "sound_system.h"
#include "null_sound_system.h"
#include <memory>

class servicelocator final
{
public:
    static sound_system& get_sound_system() { return *_ss_instance; }

    static void register_sound_system(std::unique_ptr<sound_system>&& ss)
    {
        _ss_instance = ss == nullptr
            ? std::make_unique<null_sound_system>()
            : std::move(ss);
    }

private:
    static std::unique_ptr<sound_system> _ss_instance;
};