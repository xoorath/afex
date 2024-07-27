#pragma once

// Engine
#include <Application/Application.export.h>
#include <Application/Application.h>
#include <Core/Paths.h>

//////////////////////////////////////////////////////////////////////////
#define AFEX_DECLARE_ENTRY_POINT(appType)   \
    int main(int argc, char* argv[])        \
    {                                       \
        Core::Paths paths(argc, argv);      \
        appType app;                        \
        return app.Run();                   \
    }