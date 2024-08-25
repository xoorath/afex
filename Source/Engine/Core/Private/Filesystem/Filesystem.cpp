#include <Core/Filesystem/Filesystem.h>

// Internal
#include "FilesystemImpl.h"

// System
#include <string_view>

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    /*explicit CORE_EXPORT*/ Filesystem::Filesystem(Core::Config& filesystemConfig)
        : m_PIMPL(filesystemConfig)
    {
    }

    /*CORE_EXPORT*/ Filesystem::Filesystem(const Filesystem&) = default;
    /*CORE_EXPORT*/ Filesystem::Filesystem(Filesystem&&) noexcept = default;
    /*CORE_EXPORT*/ Filesystem& Filesystem::operator=(const Filesystem&) = default;
    /*CORE_EXPORT*/ Filesystem& Filesystem::operator=(Filesystem&&) noexcept = default;
    /*CORE_EXPORT*/ Filesystem::~Filesystem() = default;

    /*CORE_EXPORT*/ ResolvePathResult Filesystem::ResolvePath(const fs::path& pathIn, fs::path& outPath) const
    {
        return m_PIMPL->ResolvePath(pathIn, outPath);
    }
}