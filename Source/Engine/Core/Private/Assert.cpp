#include <Core/Assert.h>

// System - Windows
#if WIN32
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

//////////////////////////////////////////////////////////////////////////
namespace Core::Internal
{
    //////////////////////////////////////////////////////////////////////////
    /*CORE_EXPORT*/ DevAssertionResponse HandleAssertion(const std::string& conditionText, const std::string& message)
    {
#if WIN32
        int response = MessageBox(
            /*hWND*/NULL,
            std::format(
                "Assert({}) failed.\n"
                "{}\n\n"
                "Cancel == Break In Debugger\n"
                "Try Again == Ignore Once\n"
                "Continue == Ignore Always\n",
                conditionText,
                message
            ).c_str(),
            std::format("Assert({}) failed", conditionText).c_str(),
            MB_CANCELTRYCONTINUE    // [cancel, try again, continue]
            | MB_ICONSTOP           // stopsign as the icon
            | MB_SETFOREGROUND      // bring to the front
            | MB_TASKMODAL          // take focus from all other windows in this process
            | MB_DEFBUTTON2         // default: retry 
        );

        switch (response)
        {
        default:
        case IDCANCEL:
            return DevAssertionResponse::BreakInDebugger;
        case IDTRYAGAIN:
            return DevAssertionResponse::IgnoreOnce;
        case IDCONTINUE:
            return DevAssertionResponse::IgnoreAlways;
            break;
        }
#else
        return DevAssertionResponse::BreakInDebugger;
#endif
    }
}