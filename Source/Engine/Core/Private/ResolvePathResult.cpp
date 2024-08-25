#include <Core/Filesystem/ResolvePathResult.h>

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*CORE_EXPORT*/ bool ResolvePathResultIsError(ResolvePathResult result)
    {
        switch (result)
        {
        default:
            AFEX_ASSERT_FAIL("Unhandled resolve path result");
        case ResolvePathResult::ErrorEmpty:
        case ResolvePathResult::ErrorInvalid:
        case ResolvePathResult::ErrorMupltipleVariables:
        case ResolvePathResult::ErrorVariablePlacement:
        case ResolvePathResult::ErrorVariableConcatenated:
            return true;
        case ResolvePathResult::Resolved:
        case ResolvePathResult::ResolvedFound:
        case ResolvePathResult::ResolvedNotFound:
            return false;
        }
    }

    /*CORE_EXPORT*/ const std::string_view ResolvePathResultToString(ResolvePathResult result)
    {
        using namespace std::string_view_literals;
        switch (result)
        {
        case ResolvePathResult::ErrorEmpty:
            return "error: path is empty"sv;
        case ResolvePathResult::ErrorInvalid:
            return "error: path is invalid"sv;
        case ResolvePathResult::ErrorMupltipleVariables:
            return "error: multiple variables in path"sv;
        case ResolvePathResult::ErrorVariablePlacement:
            return "error: placement of variables in path is invalid"sv;
        case ResolvePathResult::ErrorVariableConcatenated:
            return "error: variable in path is joined to more text"sv;
        case ResolvePathResult::Resolved:
            return "path valid"sv;
        case ResolvePathResult::ResolvedFound:
            return "path valid and found"sv;
        case ResolvePathResult::ResolvedNotFound:
            return "path valid and not found"sv;
        default:
            AFEX_ASSERT_FAIL("Unhandled resolve path result");
            return "UNHANDLED"sv;

        }
    }
}