#pragma once

// Engine
#include <Core/Core.export.h>
#include <Core/Flags.h>

// System
#include <string_view>

//////////////////////////////////////////////////////////////////////////
namespace Core::Text
{
    //////////////////////////////////////////////////////////////////////////
    // Each of these rule sets can be used to order strings with the StrCmp
    // function and the Comparitor class.
    // 
    // Performance notes: 
    // * Comparisons performed using std::sort of 10000 randomly generated
    //   100 character alphanumeric strings.
    // * Test was run on an AMD Ryzen 7 5800x
    //                                      Time in debug      | Time in release (sec)
    //  Default:                            0.0206595 seconds  | 0.0013665 seconds
    //  CaseInsensitive:                    0.341662 seconds   | 0.0669044 seconds
    //  NumericOrdering:                    0.0797142 seconds  | 0.002251 seconds
    //  CaseInsensitive|NumericOrdering:    0.374001 seconds   | 0.0714513 seconds
    enum class OrderingRules : uint8_t
    {
        // Standard by-character ordering based on the value of bytes in each
        // string. Does not have any special casing for non-latin1 characters,
        // casing rules or numbers.
        //
        // Example: "A" < "B" < "a1" < "a10" < "a2" < "b"
        Default = 0b0000'0000,

        // Orders basic latin letters alphabetically by their base character 
        // first and then by accented characters. Punctuation marks are
        // generally ordered after letters and numbers are treated 
        // lexicographically (character by character).
        //
        // This ordering rule is implemented with ICU and is the only ordering
        // rule set expected to work with utf-8 strings.
        // 
        // Example: "apple", "Apple", "ápple", "banana", "Banana"
        CaseInsensitive = 0b00000001,

        // Contiguous sets of numbers are ordered in groups by value.
        //
        // Example: "a1", "a2", "a3", "a10", "a11", "a30"
        NumericOrdering = 0b00000010
    };
    AFEX_FLAGS_OPERATORS(OrderingRules)

    template<OrderingRules k_OrderingRules = OrderingRules::Default>
    CORE_EXPORT int32_t StrCmp(const std::string_view& lhs, const std::string_view& rhs);

    template<OrderingRules k_OrderingRules = OrderingRules::Default>
    struct Comparitor
    {
        inline bool operator()(const std::string_view& lhs, const std::string_view& rhs) const
        {
            return StrCmp<k_OrderingRules>(lhs, rhs) < 0;
        }
    };
    
    CORE_EXPORT bool Contains(const std::string_view& lhs, const std::string_view& rhs);
    CORE_EXPORT bool ContainsCaseInsensitive(const std::string_view& lhs, const std::string_view& rhs);

    CORE_EXPORT std::string_view Trim(std::string_view viewIn);
    CORE_EXPORT std::string_view TrimLeft(std::string_view viewIn);
    CORE_EXPORT std::string_view TrimRight(std::string_view viewIn);

    // Converts a utf-16 encoded wide string to a utf-8 encoded string
    CORE_EXPORT std::string string_cast(const std::wstring_view wstringView);

}