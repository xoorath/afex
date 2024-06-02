#include "bgfxCallbacks.h"

// Engine
#include <Core/Logging.h>

// External
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>
#include <bx/file.h>
#include <bx/string.h>

namespace Graphics
{
    static const char* CodeToStr(bgfx::Fatal::Enum code)
    {
        switch (code)
        {
        case bgfx::Fatal::DebugCheck: return "DebugCheck";
        case bgfx::Fatal::InvalidShader: return "InvalidShader";
        case bgfx::Fatal::UnableToInitialize: return "UnableToInitialize";
        case bgfx::Fatal::UnableToCreateTexture: return "UnableToCreateTexture";
        case bgfx::Fatal::DeviceLost: return "DeviceLost";
        case bgfx::Fatal::Count:
        default: return "UNKNOWN";
        }
    }

    void Callbacks::fatal(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* str) /*override final*/
    {
        auto logger = Core::g_Logger;
        spdlog::source_loc loc(filePath, line, __FUNCTION__);

        // Trim off any newline (bgfx tends to end messages with newlines)
        int32_t len = bx::strLen(str);
        char temp[2048];
        bx::memCopy(temp, str, bx::min(sizeof(temp)-1, len));
        len = bx::strLen(temp, len);
        if(len > 0 && temp[len-1] == '\n')
        {
            temp[len-1] = '\0';
        }

        logger->log(loc, spdlog::level::critical, "{}: {}", CodeToStr(code), str);
    }

    void Callbacks::traceVargs(const char* filePath, uint16_t line, const char* format, va_list argList) /*override final*/
    {
        char temp[2048];
        char* out = temp;
        va_list argListCopy;
        va_copy(argListCopy, argList);
        va_end(argListCopy);
        int32_t total = bx::vsnprintf(out, sizeof(temp)-1, format, argList);
        out[total] = '\0';
        // trim off any newline (bgfx tends to end messages with newlines)
        if(out[total-1] == '\n')
        {
            out[total-1] = '\0';
        }
        spdlog::source_loc loc(filePath, line, __FUNCTION__);
        
        auto logger = Core::g_Logger;
        logger->log(loc, spdlog::level::trace, "{}", out);
    }

    void Callbacks::profilerBegin(const char* name, uint32_t abgr, const char* filePath, uint16_t line) /*override final*/
    {
        AFEX_UNUSED(name);
        AFEX_UNUSED(abgr);
        AFEX_UNUSED(filePath);
        AFEX_UNUSED(line);
    }

    void Callbacks::profilerBeginLiteral(const char* name, uint32_t abgr, const char* filePath, uint16_t line) /*override final*/
    {
        AFEX_UNUSED(name);
        AFEX_UNUSED(abgr);
        AFEX_UNUSED(filePath);
        AFEX_UNUSED(line);
    }

    void Callbacks::profilerEnd() /*override final*/
    {
    }

    uint32_t Callbacks::cacheReadSize(uint64_t id) /*override final*/
    {
        AFEX_UNUSED(id);
        return 0;
    }

    bool Callbacks::cacheRead(uint64_t id, void* data, uint32_t size) /*override final*/
    {
        AFEX_UNUSED(id);
        AFEX_UNUSED(data);
        AFEX_UNUSED(size);
        return false;
    }

    void Callbacks::cacheWrite(uint64_t id, const void* data, uint32_t size) /*override final*/
    {
        AFEX_UNUSED(id);
        AFEX_UNUSED(data);
        AFEX_UNUSED(size);
    }

    void Callbacks::screenShot(const char* filePath, uint32_t width, uint32_t height, uint32_t pitch, const void* data, uint32_t size, bool yflip) /*override final*/
    {
        AFEX_UNUSED(filePath);
        AFEX_UNUSED(width);
        AFEX_UNUSED(height);
        AFEX_UNUSED(pitch);
        AFEX_UNUSED(data);
        AFEX_UNUSED(size);
        AFEX_UNUSED(yflip);

        const int32_t len = bx::strLen(filePath) + 5;
        char* fullFilePath = (char*)alloca(len);
        bx::strCopy(fullFilePath, len, filePath);
        bx::strCat(fullFilePath, len, ".tga");

        bx::FileWriter writer;
        if (bx::open(&writer, filePath))
        {
            bimg::imageWriteTga(&writer, width, height, pitch, data, false, yflip);
            bx::close(&writer);
        }
    }

    void Callbacks::captureBegin(uint32_t width, uint32_t height, uint32_t pitch, bgfx::TextureFormat::Enum format, bool yflip) /*override final*/
    {
        AFEX_UNUSED(width);
        AFEX_UNUSED(height);
        AFEX_UNUSED(pitch);
        AFEX_UNUSED(format);
        AFEX_UNUSED(yflip);
    }

    void Callbacks::captureEnd() /*override final*/
    {
    }

    void Callbacks::captureFrame(const void* data, uint32_t size) /*override final*/
    {
        AFEX_UNUSED(data);
        AFEX_UNUSED(size);
    }
}