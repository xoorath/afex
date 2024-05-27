#pragma once

////////////////////////////////////////////////////////////////////////// Language / compiler helpers
#define AFEX_UNUSED(x) ((void)x)

////////////////////////////////////////////////////////////////////////// Debugger related
#if defined(WIN32) && !defined(_APISETDEBUG_)
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent(void); // Forward decl to avoid including Windows.h
#endif

#if _DEBUG
#define AFEX_DEBUG_BREAK() do { if(IsDebuggerPresent()) { __debugbreak(); } } while(false);
#else
#define AFEX_DEBUG_BREAK()
#endif