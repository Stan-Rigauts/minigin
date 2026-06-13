#pragma once
#include <string>
#include <memory>
#include "Component.h"
#include "Texture2D.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "GameObject.h"
#include "Transform.h"
#include <SDL3/SDL.h>


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

            if (m_texture)
            {
                float w{}, h{};
                SDL_GetTextureSize(m_texture->GetSDLTexture(), &w, &h);

                m_width = w;
                m_height = h;
            }
        }


        void SetColor(SDL_Color color)
        {
            m_color = color;
            m_useColor = true;
        }

        void SetSize(float w, float h)
        {
            m_width = w;
            m_height = h;
        }

        void SetSourceRect(int x, int y, int w, int h)
        {
            m_srcRect = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h) };
            m_useSrcRect = true;
        }

        void ClearSourceRect()
        {
            m_useSrcRect = false;
        }

        void SetCentered(bool centered)
        {
            m_Centered = centered;
        }

        void Render() const override
        {
            auto pos = GetOwner().GetWorldPosition();
            auto& renderer = Renderer::GetInstance();

            float drawX = pos.x;
            float drawY = pos.y;


            if (m_Centered)
            {
                drawX -= m_width * 0.5f;
                drawY -= m_height * 0.5f;
            }

            if (m_useColor)
            {
                SDL_FRect rect{
                    drawX,
                    drawY,
                    m_width,
                    m_height
                };

                SDL_SetRenderDrawColor(
                    renderer.GetSDLRenderer(),
                    m_color.r,
                    m_color.g,
                    m_color.b,
                    m_color.a
                );

                SDL_RenderFillRect(renderer.GetSDLRenderer(), &rect);
                return;
            }

            if (m_texture)
            {
                if (m_useSrcRect)
                {
                    renderer.RenderTexture(
                        *m_texture,
                        m_srcRect,
                        drawX,
                        drawY,
                        m_width,
                        m_height
                    );
                }
                else
                {
                    renderer.RenderTexture(
                        *m_texture,
                        drawX,
                        drawY,
                        m_width,
                        m_height
                    );
                }
            }
        }

        void SetLayer(int layer)
        {
            m_layer = layer;
        }

        int GetLayer() const
        {
            return m_layer;
        }


        float GetWidth()  const { return m_width; }
        float GetHeight() const { return m_height; }
        bool  UsesSrcRect() const { return m_useSrcRect; }
        SDL_FRect GetSrcRect() const { return m_srcRect; }

    private:

        std::shared_ptr<Texture2D> m_texture{};

        SDL_Color m_color{ 255,255,255,255 };
        bool m_useColor{ false };

        SDL_FRect m_srcRect{ 0.f, 0.f, 0.f, 0.f };
        bool m_useSrcRect{ false };

        float m_width{ 16 };
        float m_height{ 16 };
        bool m_Centered = false;
        int m_layer = 0;
    };
}