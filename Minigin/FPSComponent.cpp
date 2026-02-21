#include "FPSComponent.h"
#include "TextComponent.h"
#include <sstream>
#include <iomanip>

namespace dae
{

    dae::FPSComponent::FPSComponent(GameObject& owner)
        : Component(owner)
        , m_accumulator(0.f)
        , m_frameCount(0)
    {
    }
    void FPSComponent::Update(float delta_sec)
    {
        m_accumulator += delta_sec;
        m_frameCount++;

        if (m_accumulator >= 1.0f)
        {
            float fps = static_cast<float>(m_frameCount) / m_accumulator;

             auto textComp = GetOwner().GetComponent<TextComponent>();
            if (textComp)
            {
                std::stringstream string;
                string << std::fixed << std::setprecision(1) 
                    << fps << " FPS";
                textComp->SetText(string.str());
            }

             m_accumulator = 0.f;
            m_frameCount = 0;
        }
    }

}
