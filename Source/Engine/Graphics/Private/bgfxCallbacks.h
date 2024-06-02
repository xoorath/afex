#pragma once

// Engine
#include <Graphics/Graphics.export.h>

// External
#include <bgfx/bgfx.h>

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    //////////////////////////////////////////////////////////////////////////
    class Callbacks : public bgfx::CallbackI
    {
    public:
        Callbacks()                                     = default;
        Callbacks(const Callbacks&)                     = default;
        Callbacks(Callbacks&&) noexcept                 = default;
        Callbacks& operator=(const Callbacks&)          = default;
        Callbacks& operator=(Callbacks&&) noexcept      = default;
        ~Callbacks() final                              = default;

    /*internal:*/
        void fatal(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* str) override final;
        void profilerBegin(const char* name, uint32_t abgr, const char* filePath, uint16_t line) override final;
        void profilerBeginLiteral(const char* name, uint32_t abgr, const char* filePath, uint16_t line) override final;
        void profilerEnd() override final;
        uint32_t cacheReadSize(uint64_t id) override final;
        bool cacheRead(uint64_t id, void* data, uint32_t size) override final;
        void cacheWrite(uint64_t id, const void* data, uint32_t size) override final;
        void screenShot(const char* filePath, uint32_t width, uint32_t height, uint32_t pitch, const void* data, uint32_t size, bool yflip) override final;
        void captureBegin(uint32_t width, uint32_t height, uint32_t pitch, bgfx::TextureFormat::Enum format, bool yflip) override final;
        void captureEnd() override final;
        void captureFrame(const void* data, uint32_t size) override final;
        void traceVargs(const char* filePath, uint16_t line, const char* format, va_list argList) override final;
    };
}