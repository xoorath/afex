#pragma once

// System
#include <cstdint>

//////////////////////////////////////////////////////////////////////////
namespace Graphics
{
    //////////////////////////////////////////////////////////////////////////
    // The first member of each event type should be the message type. 
    // This will allow us to queue any event type and first cast it to 
    // MessageDataType. Once the type has been determined we can cast 
    // to the concrete type to get the data for that event.
    enum class EventType : uint8_t
    {
        Exit,
        Resize
    };

    //////////////////////////////////////////////////////////////////////////
    struct ExitEvent
    {
        EventType m_Type = EventType::Exit;
    };

    //////////////////////////////////////////////////////////////////////////
    struct ResizeEvent
    {
    public:
        ResizeEvent(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) { }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
    private:
        EventType m_Type = EventType::Resize;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };
}
