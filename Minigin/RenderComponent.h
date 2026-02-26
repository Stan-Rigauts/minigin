#pragma once
#include <string>
#include <memory>
#include "Component.h"
#include "Texture2D.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "GameObject.h"
#include "Transform.h"

namespace dae
{
    class RenderComponent final : public Component
    {
    public:
        RenderComponent(GameObject& owner)
            : Component(owner)
        {
        }

        void SetTexture(const std::string& filename)
        {
            m_texture = ResourceManager::GetInstance().LoadTexture(filename);
        }

        void Render() const override
        {
            if (!m_texture)
                return;

            auto pos = GetOwner().GetWorldPosition();
            
            Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
        }

    private:
        std::shared_ptr<Texture2D> m_texture{};
    };
}
