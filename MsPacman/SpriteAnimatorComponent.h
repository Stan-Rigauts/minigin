#pragma once
#include "Component.h"
#include <string>
#include <unordered_map>

namespace dae
{
    struct Animation
    {
        std::string Name;
        int StartFrame;
        int EndFrame;
        int Row;
        float FrameDuration;
        bool Loop;
        std::string Texture; // empty = use base texture
    };

    class SpriteAnimatorComponent final : public Component
    {
    public:
        SpriteAnimatorComponent(GameObject& owner, int frameWidth, int frameHeight);

        void AddAnimation(const Animation& anim);
        void Play(const std::string& name);
        void Update(float dt) override;

        void SetBaseTexture(const std::string& path) { m_BaseTexture = path; }
        void SetRenderSize(float w, float h) { m_RenderWidth = w; m_RenderHeight = h; }
        void SetPaused(bool paused) { m_Paused = paused; }

    private:
        void ApplyFrame();

        int m_FrameWidth;
        int m_FrameHeight;

        std::unordered_map<std::string, Animation> m_Animations;
        Animation* m_CurrentAnimation{ nullptr };
        int m_CurrentFrame{ 0 };
        float m_Timer{ 0.f };

        std::string m_BaseTexture;
        std::string m_CurrentTexture;

        float m_RenderWidth{ 28.f };
        float m_RenderHeight{ 28.f };

        bool m_Paused{ false };
    };
}