#pragma once

// Engine
#include <Application/Application.export.h>
#include <Application/Application.h>

//////////////////////////////////////////////////////////////////////////
#define AFEX_DECLARE_ENTRY_POINT(appType)   \
    int main(int argc, const char* argv[])        \
    {                                       \
        appType app;                        \
        return app.Run(argc, argv);         \
    }