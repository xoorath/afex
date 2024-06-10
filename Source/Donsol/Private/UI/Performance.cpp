#include "Performance.h"

// External
#include <imgui.h>

// System
#include <limits>
#include <string>

// External
#include <bx/bx.h>
#include <bx/string.h>
#include <bgfx/bgfx.h>

//////////////////////////////////////////////////////////////////////////
namespace Donsol::UI
{
    static bool bar(float _width, float _maxWidth, float _height, const ImVec4& _color)
    {
        const ImGuiStyle& style = ImGui::GetStyle();

        ImVec4 hoveredColor(
            _color.x + _color.x * 0.1f
            , _color.y + _color.y * 0.1f
            , _color.z + _color.z * 0.1f
            , _color.w + _color.w * 0.1f
        );

        ImGui::PushStyleColor(ImGuiCol_Button, _color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, _color);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, style.ItemSpacing.y));

        bool itemHovered = false;

        ImGui::Button("##", ImVec2(_width, _height));
        itemHovered |= ImGui::IsItemHovered();

        ImGui::SameLine();
        ImGui::InvisibleButton("##", ImVec2(bx::max(1.0f, _maxWidth - _width), _height));
        itemHovered |= ImGui::IsItemHovered();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);

        return itemHovered;
    }

    static void resourceBar(const char* _name, const char* _tooltip, uint32_t _num, uint32_t _max, float _maxWidth, float _height)
    {
        static const ImVec4 s_resourceColor(0.5f, 0.5f, 0.5f, 1.0f);
        bool itemHovered = false;

        ImGui::Text("%s: %4d / %4d", _name, _num, _max);
        itemHovered |= ImGui::IsItemHovered();
        ImGui::SameLine();

        const float percentage = float(_num) / float(_max);

        itemHovered |= bar(bx::max(1.0f, percentage * _maxWidth), _maxWidth, _height, s_resourceColor);
        ImGui::SameLine();

        ImGui::Text("%5.2f%%", percentage * 100.0f);

        if (itemHovered)
        {
            ImGui::SetTooltip("%s %5.2f%%"
                , _tooltip
                , percentage * 100.0f
            );
        }
    }

    ////////////////////////////////////////////////////////////////////////// Public
    Performance::Performance()
    {
        ResetSamples();
    }

    void Performance::Update(const bgfx::Stats* stats)
    {
        const double toMsCpu = 1000.0 / stats->cpuTimerFreq;
        const double toMsGpu = 1000.0 / stats->gpuTimerFreq;
        const double frameMs = double(stats->cpuTimeFrame) * toMsCpu;

        PushSample(static_cast<float>(frameMs));

        char frameTextOverlay[256];
        bx::snprintf(frameTextOverlay, BX_COUNTOF(frameTextOverlay), "%s%.3fms, %s%.3fms\nAvg: %.3fms, %.1f FPS"
            , "v"
            , m_Samples.m_min
            , "^"
            , m_Samples.m_max
            , m_Samples.m_avg
            , 1000.0f / m_Samples.m_avg
        );

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImColor(0.0f, 0.5f, 0.15f, 1.0f).Value);
        ImGui::PlotHistogram("Frame"
            , m_Samples.m_values
            , m_Samples.k_NumSamples
            , m_Samples.m_offset
            , frameTextOverlay
            , 0.0f
            , 60.0f
            , ImVec2(0.0f, 45.0f)
        );
        ImGui::PopStyleColor();

        ImGui::Text("Submit CPU %0.3f, GPU %0.3f (L: %d)"
            , double(stats->cpuTimeEnd - stats->cpuTimeBegin) * toMsCpu
            , double(stats->gpuTimeEnd - stats->gpuTimeBegin) * toMsGpu
            , stats->maxGpuLatency
        );

        if (-INT64_MAX != stats->gpuMemoryUsed)
        {
            char tmp0[64];
            bx::prettify(tmp0, BX_COUNTOF(tmp0), stats->gpuMemoryUsed);

            char tmp1[64];
            bx::prettify(tmp1, BX_COUNTOF(tmp1), stats->gpuMemoryMax);

            ImGui::Text("GPU mem: %s / %s", tmp0, tmp1);
        }

        if (m_showStats)
        {
            ImGui::SetNextWindowSize(
                ImVec2(300.0f, 500.0f)
                , ImGuiCond_FirstUseEver
            );

            if (ImGui::Begin("[|||] Stats", &m_showStats))
            {
                if (ImGui::CollapsingHeader("& Resources"))
                {
                    const bgfx::Caps* caps = bgfx::getCaps();

                    const float itemHeight = ImGui::GetTextLineHeightWithSpacing();
                    const float maxWidth = 90.0f;

                    //ImGui::PushFont(ImGui::Font::Mono);
                    ImGui::Text("Res: Num  / Max");
                    resourceBar("DIB", "Dynamic index buffers", stats->numDynamicIndexBuffers, caps->limits.maxDynamicIndexBuffers, maxWidth, itemHeight);
                    resourceBar("DVB", "Dynamic vertex buffers", stats->numDynamicVertexBuffers, caps->limits.maxDynamicVertexBuffers, maxWidth, itemHeight);
                    resourceBar(" FB", "Frame buffers", stats->numFrameBuffers, caps->limits.maxFrameBuffers, maxWidth, itemHeight);
                    resourceBar(" IB", "Index buffers", stats->numIndexBuffers, caps->limits.maxIndexBuffers, maxWidth, itemHeight);
                    resourceBar(" OQ", "Occlusion queries", stats->numOcclusionQueries, caps->limits.maxOcclusionQueries, maxWidth, itemHeight);
                    resourceBar("  P", "Programs", stats->numPrograms, caps->limits.maxPrograms, maxWidth, itemHeight);
                    resourceBar("  S", "Shaders", stats->numShaders, caps->limits.maxShaders, maxWidth, itemHeight);
                    resourceBar("  T", "Textures", stats->numTextures, caps->limits.maxTextures, maxWidth, itemHeight);
                    resourceBar("  U", "Uniforms", stats->numUniforms, caps->limits.maxUniforms, maxWidth, itemHeight);
                    resourceBar(" VB", "Vertex buffers", stats->numVertexBuffers, caps->limits.maxVertexBuffers, maxWidth, itemHeight);
                    resourceBar(" VL", "Vertex layouts", stats->numVertexLayouts, caps->limits.maxVertexLayouts, maxWidth, itemHeight);
                    //ImGui::PopFont();
                }

                if (ImGui::CollapsingHeader("() Profiler"))
                {
                    if (0 == stats->numViews)
                    {
                        ImGui::Text("Profiler is not enabled.");
                    }
                    else
                    {
                        if (ImGui::BeginChild("##view_profiler", ImVec2(0.0f, 0.0f)))
                        {
                            //ImGui::PushFont(ImGui::Font::Mono);

                            ImVec4 cpuColor(0.5f, 1.0f, 0.5f, 1.0f);
                            ImVec4 gpuColor(0.5f, 0.5f, 1.0f, 1.0f);

                            const float itemHeight = ImGui::GetTextLineHeightWithSpacing();
                            const float itemHeightWithSpacing = ImGui::GetFrameHeightWithSpacing();
                            const double toCpuMs = 1000.0 / double(stats->cpuTimerFreq);
                            const double toGpuMs = 1000.0 / double(stats->gpuTimerFreq);
                            const float  scale = 3.0f;

                            if (ImGui::BeginListBox("Encoders", ImVec2(ImGui::GetWindowWidth(), stats->numEncoders * itemHeightWithSpacing)))
                            {
                                ImGuiListClipper clipper;
                                clipper.Begin(stats->numEncoders, itemHeight);

                                while (clipper.Step())
                                {
                                    for (int32_t pos = clipper.DisplayStart; pos < clipper.DisplayEnd; ++pos)
                                    {
                                        const bgfx::EncoderStats& encoderStats = stats->encoderStats[pos];

                                        ImGui::Text("%3d", pos);
                                        ImGui::SameLine(64.0f);

                                        const float maxWidth = 30.0f * scale;
                                        const float cpuMs = float((encoderStats.cpuTimeEnd - encoderStats.cpuTimeBegin) * toCpuMs);
                                        const float cpuWidth = bx::clamp(cpuMs * scale, 1.0f, maxWidth);

                                        if (bar(cpuWidth, maxWidth, itemHeight, cpuColor))
                                        {
                                            ImGui::SetTooltip("Encoder %d, CPU: %f [ms]"
                                                , pos
                                                , cpuMs
                                            );
                                        }
                                    }
                                }

                                ImGui::EndListBox();
                            }

                            ImGui::Separator();

                            if (ImGui::BeginListBox("Views", ImVec2(ImGui::GetWindowWidth(), stats->numViews * itemHeightWithSpacing)))
                            {
                                ImGuiListClipper clipper;
                                clipper.Begin(stats->numViews, itemHeight);

                                while (clipper.Step())
                                {
                                    for (int32_t pos = clipper.DisplayStart; pos < clipper.DisplayEnd; ++pos)
                                    {
                                        const bgfx::ViewStats& viewStats = stats->viewStats[pos];

                                        ImGui::Text("%3d %3d %s", pos, viewStats.view, viewStats.name);

                                        const float maxWidth = 30.0f * scale;
                                        const float cpuTimeElapsed = float((viewStats.cpuTimeEnd - viewStats.cpuTimeBegin) * toCpuMs);
                                        const float gpuTimeElapsed = float((viewStats.gpuTimeEnd - viewStats.gpuTimeBegin) * toGpuMs);
                                        const float cpuWidth = bx::clamp(cpuTimeElapsed * scale, 1.0f, maxWidth);
                                        const float gpuWidth = bx::clamp(gpuTimeElapsed * scale, 1.0f, maxWidth);

                                        ImGui::SameLine(64.0f);

                                        if (bar(cpuWidth, maxWidth, itemHeight, cpuColor))
                                        {
                                            ImGui::SetTooltip("View %d \"%s\", CPU: %f [ms]"
                                                , pos
                                                , viewStats.name
                                                , cpuTimeElapsed
                                            );
                                        }

                                        ImGui::SameLine();
                                        if (bar(gpuWidth, maxWidth, itemHeight, gpuColor))
                                        {
                                            ImGui::SetTooltip("View: %d \"%s\", GPU: %f [ms]"
                                                , pos
                                                , viewStats.name
                                                , gpuTimeElapsed
                                            );
                                        }
                                    }
                                }

                                ImGui::EndListBox();
                            }

                            //ImGui::PopFont();
                        }

                        ImGui::EndChild();
                    }
                }
            }
            ImGui::End();
        }
    }

    void Performance::ResetSamples()
    {
        m_Samples.m_offset = 0;
        memset(m_Samples.m_values, 0, sizeof(m_Samples.m_values));
        m_Samples.m_min = 0.0f;
        m_Samples.m_max = 0.0f;
        m_Samples.m_avg = 0.0f;
    }

    void Performance::PushSample(float value)
    {
        m_Samples.m_values[m_Samples.m_offset] = value;
        m_Samples.m_offset = (m_Samples.m_offset + 1) % m_Samples.k_NumSamples;

        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::min();
        float avg = 0.0f;

        for (uint32_t ii = 0; ii < m_Samples.k_NumSamples; ++ii)
        {
            const float val = m_Samples.m_values[ii];
            min = val < min ? val : min;
            max = val > max ? val : max;
            avg += val;
        }

        m_Samples.m_min = min;
        m_Samples.m_max = max;
        m_Samples.m_avg = avg / m_Samples.k_NumSamples;
    }
}