#pragma once

#include "sound_system.h"

#include <memory>
#include <string>

class sdl_sound_system final : public sound_system
{
public:
    sdl_sound_system();
    ~sdl_sound_system() override;

    sdl_sound_system(const sdl_sound_system&) = delete;
    sdl_sound_system& operator=(const sdl_sound_system&) = delete;
    sdl_sound_system(sdl_sound_system&&) = delete;
    sdl_sound_system& operator=(sdl_sound_system&&) = delete;

    void play(const sound_id id, const float volume) override;
    void load(const sound_id id, const std::string& path) override;

private:
    struct impl;
    std::unique_ptr<impl> _impl;
};