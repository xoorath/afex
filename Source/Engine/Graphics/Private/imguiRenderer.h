#pragma once

// System
#include <cstdint>

struct ImGuiContext;

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    //////////////////////////////////////////////////////////////////////////
    class ImGuiRenderer
    {
    public:
        ImGuiRenderer();
        ImGuiRenderer(ImGuiRenderer&&) noexcept;
        ImGuiRenderer& operator=(ImGuiRenderer&&) noexcept;
        ~ImGuiRenderer();
        void Init(ImGuiContext* context, uint32_t width, uint32_t height);
        void Shutdown();

        void BeginFrame() const;
        void EndFrame() const;
        void Render() const; // <- Call from render thread

        void Resize(uint32_t width, uint32_t height);

        ImGuiRenderer(const ImGuiRenderer&)             = delete;
        ImGuiRenderer& operator=(const ImGuiRenderer&)  = delete;
    private:
        void* m_PIMPL = nullptr;
    };
}