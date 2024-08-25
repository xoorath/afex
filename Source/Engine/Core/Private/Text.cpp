#include <Core/Text.h>

// Engine
#include <Core/Assert.h>
#include <Core/Logging.h>

// System
#include <memory>
#include <string_view>

// External
#include <unicode/bytestream.h>
#include <unicode/coll.h>
#include <unicode/locid.h>
#include <unicode/stsearch.h>
#include <unicode/tblcoll.h>
#include <unicode/ucol.h>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>

//////////////////////////////////////////////////////////////////////////
namespace
{
    std::unique_ptr<icu::Collator> CollatorFactory()
    {
        UErrorCode status = U_ZERO_ERROR;
        std::unique_ptr<icu::Collator> collator(icu::Collator::createInstance(status));

        if (U_FAILURE(status))
        {
            AFEX_ASSERT_FAIL("AFEX Core Text failed to create an ICU Collator. "
                "\nError name: {}\nError code: {}",
                u_errorName(status),
                static_cast<int32_t>(status));
            collator.reset();
        }
        else
        {
            if (// Indicates an ICU warning:
                status != U_ZERO_ERROR 
                // Warnings we don't care about:
                && status != U_USING_DEFAULT_WARNING)
            {
                AFEX_LOG_WARNING("AFEX Core Text created an ICU Collator but received a warning."
                    "\nError name: {}\nError code: {}",
                    u_errorName(status),
                    static_cast<int32_t>(status));
            }
        }
        return std::move(collator);
    }

    bool CollatorCompare(const std::string_view& lhs, const std::string_view& rhs, const std::unique_ptr<icu::Collator>& collator, int32_t& outResult)
    {
        if (collator != nullptr)
        {
            icu::UnicodeString ulhs = icu::UnicodeString::fromUTF8(icu::StringPiece(lhs.data(), static_cast<int32_t>(lhs.length())));
            icu::UnicodeString urhs = icu::UnicodeString::fromUTF8(icu::StringPiece(rhs.data(), static_cast<int32_t>(rhs.length())));
            UErrorCode status = U_ZERO_ERROR;
            UCollationResult result = collator->compare(ulhs, urhs, status);
            if (!U_FAILURE(status))
            {
                outResult = static_cast<int32_t>(result);
                return true;
            }
            AFEX_ASSERT_FAIL("AFEX Core Text failed to compare two strings with an ICU Collator. This is unexpected."
                "\nError name: {}\nError code: {}",
                u_errorName(status),
                static_cast<int32_t>(status));
        }

        return false;
    }
}

//////////////////////////////////////////////////////////////////////////
namespace Core::Text
{
    ////////////////////////////////////////////////////////////////////////// Public
    template<>
    CORE_EXPORT int32_t StrCmp<OrderingRules::Default>(const std::string_view& lhs, const std::string_view& rhs)
    {
        return lhs.compare(rhs);
    }

    template<>
    CORE_EXPORT int32_t StrCmp<OrderingRules::CaseInsensitive>(const std::string_view& lhs, const std::string_view& rhs)
    {
        using namespace std::string_view_literals;
        thread_local struct CollatorWrapper
        {
            CollatorWrapper() : collator(CollatorFactory())
            {
            }

            std::unique_ptr<icu::Collator> collator;
        } s_CollatorWrapper;
        int32_t result;
        if (CollatorCompare(lhs, rhs, s_CollatorWrapper.collator, result))
        {
            return result;
        }
        return StrCmp<OrderingRules::Default>(lhs, rhs);
    }

    template<>
    CORE_EXPORT int32_t StrCmp<OrderingRules::NumericOrdering>(const std::string_view& lhs, const std::string_view& rhs)
    {
        std::string_view::const_iterator lit = lhs.cbegin();
        std::string_view::const_iterator rit = rhs.cbegin();

        while(lit != lhs.cend() && rit != rhs.cend())
        {
            std::string_view::value_type lchar = *lit;
            std::string_view::value_type rchar = *rit;

            // both values are digits
            if(lchar >= '0' && lchar <= '9' 
            && rchar >= '0' && rchar <= '9')
            {
                // extract and compare the sequence
                uint64_t lnum = 0;
                uint64_t rnum = 0;
                do
                {
                    lchar = *lit;
                    lnum = lnum * 10 + (lchar - '0');
                    lit++;
                }
                while(lit != lhs.cend() && lchar >= '0' && lchar <= '9');

                do
                {
                    rchar = *rit;
                    rnum = rnum * 10 + (rchar - '0');
                    rit++;
                } while (rit != rhs.cend() && rchar >= '0' && rchar <= '9');

                if(lnum < rnum) return -1;
                if(lnum > rnum) return 1;
            }
            else
            {
                // Compare normally if either or both characters are not numeric
                if(lchar < rchar) return -1;
                if(lchar > rchar) return 1;
                lit++;
                rit++;
            }
        }

        if (lit != lhs.cend()) return 1;
        if (rit != rhs.cend()) return -1;
        return 0;
    }

    template<>
    CORE_EXPORT int32_t StrCmp<OrderingRules::CaseInsensitive|OrderingRules::NumericOrdering>(const std::string_view& lhs, const std::string_view& rhs)
    {
        using namespace std::string_view_literals;
        thread_local struct CollatorWrapper
        {
            CollatorWrapper() : collator(CollatorFactory())
            {
                UErrorCode status = U_ZERO_ERROR;
                collator->setAttribute(UCOL_NUMERIC_COLLATION, UCOL_ON, status); // No special case ordering

                if (U_FAILURE(status))
                {
                    AFEX_ASSERT_FAIL("AFEX Core Text failed to set an attribute on an ICU Collator. "
                        "\nError name: {}\nError code: {}",
                        u_errorName(status),
                        static_cast<int32_t>(status));
                    collator.reset();
                }
            }

            std::unique_ptr<icu::Collator> collator;
        } s_CollatorWrapper;
        int32_t result;
        if (CollatorCompare(lhs, rhs, s_CollatorWrapper.collator, result))
        {
            return result;
        }
        return StrCmp<OrderingRules::Default>(lhs, rhs);
    }

    // Explicit declarations for linking
    template CORE_EXPORT int32_t StrCmp<OrderingRules::Default>(const std::string_view& lhs, const std::string_view& rhs);
    template CORE_EXPORT int32_t StrCmp<OrderingRules::CaseInsensitive>(const std::string_view& lhs, const std::string_view& rhs);
    template CORE_EXPORT int32_t StrCmp<OrderingRules::NumericOrdering>(const std::string_view& lhs, const std::string_view& rhs);
    template CORE_EXPORT int32_t StrCmp<OrderingRules::CaseInsensitive|OrderingRules::NumericOrdering>(const std::string_view& lhs, const std::string_view& rhs);

    /*CORE_EXPORT*/ bool Contains(const std::string_view& lhs, const std::string_view& rhs)
    {
        return lhs.find_first_of(rhs) != std::string_view::npos;
    }

    /*CORE_EXPORT*/ bool ContainsCaseInsensitive(const std::string_view& lhs, const std::string_view& rhs)
    {
        using namespace std::string_view_literals;
        thread_local struct CollatorWrapper
        {
            CollatorWrapper() : collator(CollatorFactory())
            {
            }

            std::unique_ptr<icu::Collator> collator;
        } s_CollatorWrapper;
        
        UErrorCode status;
        icu::StringSearch search(icu::UnicodeString::fromUTF8(icu::StringPiece(lhs.data(), static_cast<int32_t>(lhs.length()))),
                                 icu::UnicodeString::fromUTF8(icu::StringPiece(rhs.data(), static_cast<int32_t>(rhs.length()))),
                                 icu::Locale::getDefault(),
                                 nullptr,
                                 status);
        if (U_FAILURE(status))
        {
            AFEX_ASSERT_FAIL("AFEX Core Text failed to check of one string contains another case insensitively."
                "\nError name: {}\nError code: {}\nlhs:{}\nrhs:{}",
                u_errorName(status),
                static_cast<int32_t>(status),
                lhs, 
                rhs);
            return false;
        }
        int32_t index = search.first(status);
        return U_SUCCESS(status) && index != USEARCH_DONE;
    }

    /*CORE_EXPORT*/ std::string_view Trim(std::string_view viewIn)
    {
        using namespace std::string_view_literals;

        const size_t leftIdx = viewIn.find_first_not_of(" \t\n\r\f\v");
        if(leftIdx == std::string_view::npos)
        {
            return {};
        }

        const size_t rightIdx = viewIn.find_last_not_of(" \t\n\r\f\v");
        const size_t range = rightIdx - leftIdx + 1;
        return viewIn.substr(leftIdx, range);
    }

    /*CORE_EXPORT*/ std::string_view TrimLeft(std::string_view viewIn)
    {
        using namespace std::string_view_literals;

        const size_t leftIdx = viewIn.find_first_not_of(" \t\n\r\f\v");
        if (leftIdx == std::string_view::npos)
        {
            return {};
        }

        const size_t rightIdx = viewIn.size() - 1;
        const size_t range = rightIdx - leftIdx + 1;
        return viewIn.substr(leftIdx, range);
    }

    /*CORE_EXPORT*/ std::string_view TrimRight(std::string_view viewIn)
    {
        using namespace std::string_view_literals;

        const size_t leftIdx = 0;
        const size_t rightIdx = viewIn.find_last_not_of(" \t\n\r\f\v");
        const size_t range = rightIdx - leftIdx + 1;
        return viewIn.substr(leftIdx, range);
    }

    /*CORE_EXPORT*/ std::string string_cast(const std::wstring_view wstringView)
    {
        icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(wstringView.data()), static_cast<int32_t>(wstringView.size()));

        std::vector<char> utf8Buffer(unicodeStr.length()*4);
        icu::CheckedArrayByteSink sink(utf8Buffer.data(), static_cast<int32_t>(utf8Buffer.size()));
        unicodeStr.toUTF8(sink);

        utf8Buffer.resize(sink.NumberOfBytesWritten());

        return std::string(utf8Buffer.data(), utf8Buffer.size());
    }
}