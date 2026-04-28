#pragma once
#include "sound_system.h"

class null_sound_system final : public sound_system
{
public:
    void play(const sound_id, const float) override {}
    void load(const sound_id, const std::string&) override {}
};