#include <Platform/WindowArgs.h>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*PLATFORM_EXPORT*/ WindowArgs::WindowArgs(
        std::string_view title,
        ImGuiContext* context,
        uint32_t width,
        uint32_t height)
        : m_Title(title)
        , m_ImguiContext(context)
        , m_Width(width)
        , m_Height(height)
    {
    }

    /*PLATFORM_EXPORT*/ WindowArgs::WindowArgs() = default;
    /*PLATFORM_EXPORT*/ WindowArgs::WindowArgs(const WindowArgs&) = default;
    /*PLATFORM_EXPORT*/ WindowArgs& WindowArgs::operator=(const WindowArgs&) = default;
    /*PLATFORM_EXPORT*/ WindowArgs::WindowArgs(WindowArgs&&) noexcept = default;
    /*PLATFORM_EXPORT*/ WindowArgs& WindowArgs::operator=(WindowArgs&&) noexcept = default;
    /*PLATFORM_EXPORT*/ WindowArgs::~WindowArgs() = default;

    //////////////////////////////////////////////////////////////////////////  Internal
    const std::string& WindowArgs::GetTitle() const { return m_Title; }
    ImGuiContext* WindowArgs::GetImguiContextMutable() const { return m_ImguiContext; }
    uint32_t WindowArgs::GetWidth() const { return m_Width; }
    uint32_t WindowArgs::GetHeight() const { return m_Height; }
}