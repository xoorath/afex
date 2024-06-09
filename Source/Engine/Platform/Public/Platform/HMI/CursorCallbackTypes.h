#pragma once

// Engine
#include <Core/Signal.h>

// System
#include <cstdint>

//////////////////////////////////////////////////////////////////////////
namespace Platform
{
    //////////////////////////////////////////////////////////////////////////
    using CursorPositionCallbackType    = Core::Signal<void(float xPos, float yPos)>;
    using CursorEnterCallbackType       = Core::Signal<void(int entered)>;
    using MouseButtonCallbackType       = Core::Signal<void(int button, int action, int mods)>;
    using ScrollCallbackType            = Core::Signal<void(float xScroll, float yScroll)>;

}