#pragma once
#include "Component.h"
#include "CacheTrasher.h"
#include <vector>

namespace dae
{
    class ThrashTheCacheComponent final : public Component
    {
    public:
        explicit ThrashTheCacheComponent(GameObject& owner);

        void Update(float /*dt*/) override {}
        void Render() const override;

    private:
        mutable int m_Ex1Samples;
        mutable TimingResult m_Ex1Result;
        mutable bool m_Ex1HasResult;

        mutable int m_Ex2Samples;
        mutable TimingResult m_Ex2ResultGO3D;
        mutable TimingResult m_Ex2ResultGO3DAlt;
        mutable bool m_Ex2HasResult;
        mutable bool m_Ex2AltHasResult;

        float m_Xs[11]; 
    };
}