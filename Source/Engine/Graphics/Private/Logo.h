#pragma once

// System
#include <cstdint>

//////////////////////////////////////////////////////////////////////////
namespace Graphics::Logo
{
    constexpr uint32_t k_BufferWidth = 80;
    constexpr uint32_t k_BufferHeight = 25;
    constexpr uint32_t k_BufferBytesPerPixel = 2;
    constexpr uint32_t k_BufferSize = k_BufferWidth * k_BufferHeight * k_BufferBytesPerPixel;

    // This logo is created with the dos program TheDraw.
    // The logo file is saved as binary and converted to a const uint8_t array.
    // The sizes below are hard-coded after the fact.
    extern const uint8_t k_ImageData[k_BufferSize];
    constexpr uint32_t k_ImageWidth = 52;
    constexpr uint32_t k_ImageHeight = 9;
}