#include <Graphics/RenderEngineArgs.h>

namespace Graphics
{
    /*GRAPHICS_EXPORT*/ RenderEngineArgs::RenderEngineArgs(void* nativeWindowHandle, uint32_t width, uint32_t height)
        : m_NativeWindowHandle(nativeWindowHandle)
        , m_Width(width)
        , m_Height(height)
    {
    }

    /*GRAPHICS_EXPORT*/ RenderEngineArgs::RenderEngineArgs() = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs::RenderEngineArgs(const RenderEngineArgs&) = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs& RenderEngineArgs::operator=(const RenderEngineArgs&) = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs::RenderEngineArgs(RenderEngineArgs&&) noexcept = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs& RenderEngineArgs::operator=(RenderEngineArgs&&) noexcept = default;
    /*GRAPHICS_EXPORT*/ RenderEngineArgs::~RenderEngineArgs() = default;

    /*internal*/ void* RenderEngineArgs::GetNativeWindowHandle() const
    {
        return m_NativeWindowHandle;
    }

    /*internal*/ uint32_t RenderEngineArgs::GetWidth() const
    {
        return m_Width;
    }

    /*internal*/ uint32_t RenderEngineArgs::GetHeight() const
    {
        return m_Height;
    }
}