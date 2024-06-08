#pragma once

// Engine
#include <Core/Core.export.h>

// System
#include <type_traits>

//////////////////////////////////////////////////////////////////////////
#define AFEX_FLAGS_OPERATORS(EnumType)                                  \
    inline EnumType operator | (EnumType left, EnumType right)          \
    {                                                                   \
        return static_cast<EnumType>                                    \
        (                                                               \
            static_cast<std::underlying_type<EnumType>::type>(left)     \
            | static_cast<std::underlying_type<EnumType>::type>(right)  \
        );                                                              \
    }                                                                   \
    inline EnumType operator & (EnumType left, EnumType right)          \
    {                                                                   \
        return static_cast<EnumType>                                    \
        (                                                               \
            static_cast<std::underlying_type<EnumType>::type>(left)     \
            & static_cast<std::underlying_type<EnumType>::type>(right)  \
        );                                                              \
    }                                                                   \
    inline EnumType& operator |= (EnumType& left, EnumType right)       \
    {                                                                   \
        return left = static_cast<EnumType>                             \
        (                                                               \
            static_cast<std::underlying_type<EnumType>::type>(left)     \
            | static_cast<std::underlying_type<EnumType>::type>(right)  \
        );                                                              \
    }                                                                   \
    inline EnumType& operator &= (EnumType& left, EnumType right)       \
    {                                                                   \
        return left = static_cast<EnumType>                             \
        (                                                               \
            static_cast<std::underlying_type<EnumType>::type>(left)     \
            & static_cast<std::underlying_type<EnumType>::type>(right)  \
        );                                                              \
    }