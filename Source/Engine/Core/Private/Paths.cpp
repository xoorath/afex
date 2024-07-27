#include <Core/Paths.h>

#include <Core/Assert.h>

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*CORE_EXPORT*/ Paths::Paths(int argc, char* argv[])
    {
        AFEX_ASSERT(argc > 0);
        s_ExecutablePath = fs::path(argv[0]);
        AFEX_ASSERT(s_ExecutablePath.has_parent_path());
        s_ExecutableDirectory = s_ExecutablePath.parent_path();
    }

    /*CORE_EXPORT*/ Paths::~Paths() = default;

    ////////////////////////////////////////////////////////////////////////// Public Static
    /*CORE_EXPORT static*/ std::filesystem::path Paths::ExecutableDirectory()
    {
        return s_ExecutableDirectory;
    }

    /*CORE_EXPORT static*/ std::filesystem::path Paths::ExecutablePath()
    {
        return s_ExecutablePath;
    }

    ////////////////////////////////////////////////////////////////////////// Private Static
    /*static*/ fs::path Paths::s_ExecutableDirectory;
    /*static*/ fs::path Paths::s_ExecutablePath;
}