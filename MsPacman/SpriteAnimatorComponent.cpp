#include "SpriteAnimatorComponent.h"
#include "GameObject.h"
#include "RenderComponent.h"

namespace dae
{
    SpriteAnimatorComponent::SpriteAnimatorComponent(
        GameObject& owner,
        int frameWidth,
        int frameHeight)
        : Component(owner)
        , m_FrameWidth(frameWidth)
        , m_FrameHeight(frameHeight)
    {
    }

    void SpriteAnimatorComponent::AddAnimation(const Animation& anim)
    {
        m_Animations[anim.Name] = anim;
    }

    void SpriteAnimatorComponent::Play(const std::string& name)
    {
        auto it = m_Animations.find(name);
        if (it == m_Animations.end()) return;

        if (m_CurrentAnimation == &it->second) return;

        m_CurrentAnimation = &it->second;
        m_CurrentFrame = m_CurrentAnimation->StartFrame;
        m_Timer = 0.f;

        auto render = GetOwner().GetComponent<RenderComponent>();
        if (render)
        {
            const std::string& tex = m_CurrentAnimation->Texture.empty()
                ? m_BaseTexture
                : m_CurrentAnimation->Texture;

            if (tex != m_CurrentTexture)
            {
                m_CurrentTexture = tex;
                render->SetTexture(tex);
                render->SetSize(m_RenderWidth, m_RenderHeight);
            }
        }

        ApplyFrame();
    }

    void SpriteAnimatorComponent::Update(float dt)
    {
        if (!m_CurrentAnimation || m_Paused) return;

        m_Timer += dt;
        if (m_Timer < m_CurrentAnimation->FrameDuration) return;
        m_Timer -= m_CurrentAnimation->FrameDuration;

        ++m_CurrentFrame;
        if (m_CurrentFrame > m_CurrentAnimation->EndFrame)
        {
            if (m_CurrentAnimation->Loop)
                m_CurrentFrame = m_CurrentAnimation->StartFrame;
            else
                m_CurrentFrame = m_CurrentAnimation->EndFrame;
        }

        ApplyFrame();
    }

    void SpriteAnimatorComponent::ApplyFrame()
    {
        auto render = GetOwner().GetComponent<RenderComponent>();
        if (!render || !m_CurrentAnimation) return;

        render->SetSourceRect(
            m_CurrentFrame * m_FrameWidth,
            m_CurrentAnimation->Row * m_FrameHeight,
            m_FrameWidth,
            m_FrameHeight
        );
    }
}