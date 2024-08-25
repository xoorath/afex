#pragma once

// Engine
#include <Core/Core.export.h>
#include <Core/PIMPL.h>

// System
#include <cstdint>
#include <filesystem>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    //////////////////////////////////////////////////////////////////////////
    class Config;
    class FilesystemImpl;
    enum class ResolvePathResult : uint8_t;

    //////////////////////////////////////////////////////////////////////////
    // The filesystem's primary purpose is the management of special path 
    // locations so all files (with rare exception) in the engine and game may 
    // be managed relative to one of these special locations.
    // 
    // The application's assets directory is special in that it can refer to
    // a stack of several locations in a specific order to allow devs to 
    // override files by having a higher order asset folder searched first
    // For example the configured assets directory stack might be:
    //      [1] = {{local}}/built_data/assets/
    //      [0] = {{app}}/assets/
    // In this example you can imagine a local data build process that puts
    // locally built content into the user's %LOCALAPPDATA%\<appname>\ folder
    // which the developer would want to load instead of the data packaged with
    // the application. In this case if there was a file named picture.png in
    // both locations it could be loaded with "{{assets}}/picture.png" from
    // the highest level asset location if it exists. If it did not exist the
    // lower level asset location would be searched - only failing to find if
    // it was not present in any of the configured locations.
    //
    // Special paths:
    //      The following tokens can be resolved in an asset path. For example
    //      the path "{{app}}/foo.txt" will be resolved to an absolute path 
    //      where {{app}} is replaced by the executable directory.
    // 
    //      {{cwd}} The current working directory.
    //      {{app}} The directory of the executable.
    //      {{assets}} Any of the configured asset directories. When a path
    //          with this token is resolved: if the file exists that path
    //          will be used. If the file is not found the first asset 
    //          directory will be used
    //      {{local}} Used for files specific to this user and machine.
    //          Windows:    %LOCALAPPDATA%\<appname>\
    //          Linux:      ~/.local/share/<appname>/
    //      {{roaming}} Used for files specific to this user. These files
    //          should be appropriate to synchronized across systems.
    //          Windows:    %APPDATA%\<appname>\
    //          Linux:      ~/.config/<appname>/
    //      {{temp}} A temporary directory expected to be empty at the
    //          beginning of the session. During graceful shutdown this 
    //          directory is deleted. During startup old temporary 
    //          directories will be attempted to be deleted.
    //
    //      Notes:
    //      These special tokens are case insensitive but lowercase is
    //      encouraged for consistency.
    //
    //      There is no need to escape syntax resembling these tokens: the 
    //      program will scan for these exact symbols, not for symbols with
    //      {{}} syntax.
    //
    //      It is only ever valid to use one of these special tokens in a path
    //      and it is only valid at the beginning of a path with no other text
    //      accompanying it. The following are examples of invalid paths:
    //          {{cwd}}foo/bar
    //          {{app}}/foo/{{temp}}/bar
    //          {{app}}{{temp}}/foo
    //
    // Configuration:
    //      The command line can be provided with a config file path. See
    //      the Application library for details on that.
    //      The config file can have settings for the following:
    //
    //      "Filesystem/Assets" - an array of path strings. These paths
    //      will be checked on startup. If they exist and are directories
    //      they will be used in order to look for assets. The default value
    //      is ["${app}/Assets"]. Special paths may be used.
    //
    // Special handling:
    //      1. Relative paths (. and ..) are handled and resolved to full paths.
    //      2. Paths are case sensitive even on Windows.
    //      3. Path separators are expected to be forward slashes until resolved.
    //      3.1 When resolved path separators are native.
    //      4. Directory paths will have a trailing path separator.
    //      5. Concatenating paths is tolerant of additional separators.
    //          ie: "C:/myexedir/" + "/foo.txt" == "C:\myexedir\foo.txt"
    //      6. Paths are always utf8 in AFEX.
    //      6.1 On Windows we use FILE_FLAG_POSIX_SEMANTICS to use utf8 rather
    //      than utf16. 
    class Filesystem
    {
    public:
        explicit CORE_EXPORT Filesystem(Core::Config& filesystemConfig);

        Filesystem()                                                  = delete;
        CORE_EXPORT Filesystem(const Filesystem&);                  /*= default*/
        CORE_EXPORT Filesystem(Filesystem&&) noexcept;              /*= default*/
        CORE_EXPORT Filesystem& operator=(const Filesystem&);       /*= default*/
        CORE_EXPORT Filesystem& operator=(Filesystem&&) noexcept;   /*= default*/
        CORE_EXPORT ~Filesystem();                                  /*= default*/

        // Creates an absolute path from a path that may be relative and/or contain variables.
        CORE_EXPORT ResolvePathResult ResolvePath(const std::filesystem::path& pathIn, std::filesystem::path& outPath) const;

    private:
        PIMPL<FilesystemImpl, 56> m_PIMPL;
    };
}