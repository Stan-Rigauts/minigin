#pragma once
#include "Component.h"
#include <memory>

namespace dae
{
    class TextComponent;

    class FPSComponent final : public Component
    {
    public:
        FPSComponent() = default;

        void Update(float delta_sec) override;

    private:
        float m_accumulator{ 0.f };
        int m_frameCount{ 0 };
    };
}
