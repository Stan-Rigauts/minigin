#include "sdl_sound_system.h"

#include <SDL3_mixer/SDL_mixer.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <condition_variable>

namespace
{
    enum class sound_event_type
    {
        play,
        load,
        quit
    };

    struct sound_event
    {
        sound_event_type type{};
        sound_id id{};
        float volume{ 1.0f };
        std::string path{};
    };
}

struct sdl_sound_system::impl
{
    std::unordered_map<sound_id, MIX_Audio*> audio_clips{};  
    std::queue<sound_event> event_queue{};

    std::mutex mutex{};
    std::condition_variable cv{};
    std::thread worker{};

    MIX_Mixer* mixer{ nullptr }; 

    impl()
    {
        if (!MIX_Init())  
        {
            std::cerr << "sdl_sound_system: MIX_Init failed: " << SDL_GetError() << '\n';
            return;
        }

        mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);  
        if (!mixer)
        {
            std::cerr << "sdl_sound_system: MIX_CreateMixerDevice failed: " << SDL_GetError() << '\n';
            MIX_Quit();  
            return;
        }

        worker = std::thread(&impl::process_loop, this);
    }

    ~impl()
    {
        enqueue({ sound_event_type::quit, 0, 1.0f, {} });

        if (worker.joinable())
            worker.join();

        for (auto& [id, audio] : audio_clips)
        {
            (void)id;
            if (audio)
                MIX_DestroyAudio(audio); 
        }

        if (mixer)
            MIX_DestroyMixer(mixer);  

        MIX_Quit();  
    }

    void enqueue(sound_event evt)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            event_queue.push(std::move(evt));
        }
        cv.notify_one();
    }

    void process_loop()
    {
        for (;;)
        {
            sound_event evt{};

            {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock, [this] { return !event_queue.empty(); });

                evt = std::move(event_queue.front());
                event_queue.pop();
            }

            switch (evt.type)
            {
            case sound_event_type::quit:
                return;

            case sound_event_type::load:
            {
                if (!mixer)
                {
                    std::cerr << "mixer not initialized\n";
                    break;
                }

                if (audio_clips.contains(evt.id))
                    break;

                MIX_Audio* audio = MIX_LoadAudio(mixer, evt.path.c_str(), false);
                if (!audio)
                {
                    std::cerr << "failed to load " << evt.path << ": " << SDL_GetError() << '\n';
                    break;
                }
                    
                audio_clips.emplace(evt.id, audio);
                break;
            }

            case sound_event_type::play:
            {
                if (!mixer)
                {
                    std::cerr << "mixer not initialized\n";
                    break;
                }

                const auto it = audio_clips.find(evt.id);
                if (it == audio_clips.end() || !it->second)
                {
                    std::cerr << "sound id not loaded: " << evt.id << '\n';
                    break;
                }

                if (!MIX_PlayAudio(mixer, it->second))
                {
                    std::cerr << "MIX_PlayAudio failed: " << SDL_GetError() << '\n';
                }

                break;
            }
            }
        }
    }
};

sdl_sound_system::sdl_sound_system()
    : _impl(std::make_unique<impl>())  
{
}

sdl_sound_system::~sdl_sound_system() = default;

void sdl_sound_system::play(const sound_id id, const float volume)
{
    _impl->enqueue({ sound_event_type::play, id, volume, {} });
}

void sdl_sound_system::load(const sound_id id, const std::string& path)
{
    _impl->enqueue({ sound_event_type::load, id, 1.0f, path });
}