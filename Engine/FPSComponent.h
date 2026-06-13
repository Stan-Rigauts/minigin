#pragma once
#include "Component.h"
#include <memory>

namespace dae
{
    class TextComponent;

    class FPSComponent final : public Component
    {
    public:
        FPSComponent(GameObject& owner);

        void Update(float delta_sec) override;

    private:
        float m_accumulator;
        int m_frameCount;
    };
}
