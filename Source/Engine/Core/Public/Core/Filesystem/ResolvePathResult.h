#pragma once

// Engine
#include <Core/Core.export.h>

// System
#include <cstdint>
#include <string_view>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    enum class ResolvePathResult : uint8_t
    {
        // A path to be resolved may not be empty
        ErrorEmpty,

        // One or more characters in the path are invalid
        ErrorInvalid,

        // The path contained multiple variables.
        // Only one variable is allowed and only at the beginning of a path.
        ErrorMupltipleVariables,

        // The placement of a variable is invalid.
        // Only one variable is allowed and only at the beginning of a path.
        ErrorVariablePlacement,

        // Variables may only exist alone in a path part before a separator
        // Example: "{{app}}foo/bar" is invalid. "{{app}}/foo/bar" is valid.
        ErrorVariableConcatenated,

        // The path was resolved. The existence of that file/directory was not checked. 
        Resolved,

        // The path was resolved and a file or directory exists at that location.
        // The existence of a path is checked when the path is under {{assets}}
        ResolvedFound,

        // The path was resolved and no file or directory exists at that location
        // The existence of a path is checked when the path is under {{assets}}
        ResolvedNotFound
    };

    // Returns true if the result arg is any error type
    CORE_EXPORT bool ResolvePathResultIsError(ResolvePathResult result);

    // Get a human readable string for the path result
    CORE_EXPORT const std::string_view ResolvePathResultToString(ResolvePathResult result);
}