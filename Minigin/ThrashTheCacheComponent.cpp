#include "ThrashTheCacheComponent.h"
#include <imgui.h>
#include "imgui_plot.h"

namespace dae
{
    ThrashTheCacheComponent::ThrashTheCacheComponent(GameObject& owner)
        : Component(owner)
        , m_Ex1Samples(10)
        , m_Ex1HasResult(false)
        , m_Ex2Samples(100)
        , m_Ex2HasResult(false)
        , m_Ex2AltHasResult(false)
    {
        for (int i = 0; i < 11; ++i)
            m_Xs[i] = float(i);
    }


    void ThrashTheCacheComponent::Render() const
    {
        // ───────────────────────────────────────────────
        // Exercise 1
        // ───────────────────────────────────────────────
        ImGui::Begin("Exercise 1");

        ImGui::InputInt("# samples##ex1", (int*)&m_Ex1Samples);
        if (m_Ex1Samples < 1) m_Ex1Samples = 1;

        if (ImGui::Button("Thrash the cache"))
        {
            const_cast<TimingResult&>(m_Ex1Result) =
                CacheTrasher::RunExercise1(m_Ex1Samples);
            const_cast<bool&>(m_Ex1HasResult) = true;
        }

        if (m_Ex1HasResult && !m_Ex1Result.timings.empty())
        {
            float maxVal = *std::max_element(
                m_Ex1Result.timings.begin(),
                m_Ex1Result.timings.end()
            );

            ImGui::Spacing();
            ImGui::Text("int buffer - avg us per step");

            ImGui::PlotConfig conf;
            conf.values.xs = m_Xs;
            conf.values.ys = m_Ex1Result.timings.data();
            conf.values.count = (int)m_Ex1Result.timings.size();
            conf.values.color = IM_COL32(0, 200, 100, 255);
            conf.scale.min = 0.f;
            conf.scale.max = maxVal * 1.2f;
            conf.tooltip.show = true;
            conf.tooltip.format = "step %g : %.1f us";
            conf.grid_x.show = true;
            conf.grid_y.show = true;
            conf.grid_y.size = maxVal / 4.f;
            conf.frame_size = ImVec2(ImGui::GetContentRegionAvail().x, 160);
            conf.line_thickness = 2.f;

            ImGui::Plot("##ex1plot", conf);

            ImGui::Text("x: 1 2 4 8 16 32 64 128 256 512 1024");
            for (int i = 0; i < (int)m_Ex1Result.timings.size(); ++i)
                ImGui::Text("  step %4d : %.1f us", 1 << i, m_Ex1Result.timings[i]);
        }

        ImGui::End();

        // ───────────────────────────────────────────────
        // Exercise 2
        // ───────────────────────────────────────────────
        ImGui::Begin("Exercise 2");

        ImGui::InputInt("# samples##ex2", (int*)&m_Ex2Samples);
        if (m_Ex2Samples < 1) m_Ex2Samples = 1;

        if (ImGui::Button("Thrash the cache with GameObject3D"))
        {
            const_cast<TimingResult&>(m_Ex2ResultGO3D) =
                CacheTrasher::RunExercise2_GameObject3D(m_Ex2Samples);
            const_cast<bool&>(m_Ex2HasResult) = true;
        }

        if (ImGui::Button("Thrash the cache with GameObject3DAlt"))
        {
            const_cast<TimingResult&>(m_Ex2ResultGO3DAlt) =
                CacheTrasher::RunExercise2_GameObject3DAlt(m_Ex2Samples);
            const_cast<bool&>(m_Ex2AltHasResult) = true;
        }

        if (m_Ex2HasResult || m_Ex2AltHasResult)
        {
            float maxVal = 1.f;

            if (m_Ex2HasResult && !m_Ex2ResultGO3D.timings.empty())
                maxVal = std::max(maxVal, *std::max_element(
                    m_Ex2ResultGO3D.timings.begin(),
                    m_Ex2ResultGO3D.timings.end()));

            if (m_Ex2AltHasResult && !m_Ex2ResultGO3DAlt.timings.empty())
                maxVal = std::max(maxVal, *std::max_element(
                    m_Ex2ResultGO3DAlt.timings.begin(),
                    m_Ex2ResultGO3DAlt.timings.end()));

            if (m_Ex2HasResult && m_Ex2AltHasResult)
            {
                ImGui::Text("GameObject3D (green) vs GameObject3DAlt (orange)");

                const float* ys_list[2] = {
                    m_Ex2ResultGO3D.timings.data(),
                    m_Ex2ResultGO3DAlt.timings.data()
                };

                static const ImU32 colors[2] = {
                    IM_COL32(0, 200, 100, 255),
                    IM_COL32(255, 160, 0, 255)
                };

                ImGui::PlotConfig conf;
                conf.values.xs = m_Xs;
                conf.values.ys_list = ys_list;
                conf.values.ys_count = 2;
                conf.values.colors = colors;
                conf.values.count = (int)m_Ex2ResultGO3D.timings.size();
                conf.scale.min = 0.f;
                conf.scale.max = maxVal * 1.2f;
                conf.tooltip.show = true;
                conf.tooltip.format = "step %g : %.1f us";
                conf.grid_x.show = true;
                conf.grid_y.show = true;
                conf.grid_y.size = maxVal / 4.f;
                conf.frame_size = ImVec2(ImGui::GetContentRegionAvail().x, 180);
                conf.line_thickness = 2.f;

                ImGui::Plot("##ex2both", conf);
            }
            else
            {
                bool useGO3D = m_Ex2HasResult;
                const float* ys = useGO3D ?
                    m_Ex2ResultGO3D.timings.data() :
                    m_Ex2ResultGO3DAlt.timings.data();

                int cnt = useGO3D ?
                    (int)m_Ex2ResultGO3D.timings.size() :
                    (int)m_Ex2ResultGO3DAlt.timings.size();

                ImU32 col = useGO3D ?
                    IM_COL32(0, 200, 100, 255) :
                    IM_COL32(255, 160, 0, 255);

                const char* lbl = useGO3D ?
                    "GameObject3D (pointer chase)" :
                    "GameObject3DAlt (direct)";

                ImGui::Text("%s", lbl);

                ImGui::PlotConfig conf;
                conf.values.xs = m_Xs;
                conf.values.ys = ys;
                conf.values.count = cnt;
                conf.values.color = col;
                conf.scale.min = 0.f;
                conf.scale.max = maxVal * 1.2f;
                conf.tooltip.show = true;
                conf.tooltip.format = "step %g : %.1f us";
                conf.grid_x.show = true;
                conf.grid_y.show = true;
                conf.grid_y.size = maxVal / 4.f;
                conf.frame_size = ImVec2(ImGui::GetContentRegionAvail().x, 180);
                conf.line_thickness = 2.f;

                ImGui::Plot("##ex2single", conf);
            }

            ImGui::Text("x: 1 2 4 8 16 32 64 128 256 512 1024");

            if (m_Ex2HasResult)
            {
                ImGui::Text("GameObject3D:");
                for (int i = 0; i < (int)m_Ex2ResultGO3D.timings.size(); ++i)
                    ImGui::Text("  step %4d : %.1f us", 1 << i, m_Ex2ResultGO3D.timings[i]);
            }

            if (m_Ex2AltHasResult)
            {
                ImGui::Text("GameObject3DAlt:");
                for (int i = 0; i < (int)m_Ex2ResultGO3DAlt.timings.size(); ++i)
                    ImGui::Text("  step %4d : %.1f us", 1 << i, m_Ex2ResultGO3DAlt.timings[i]);
            }
        }

        ImGui::End();
    }
}