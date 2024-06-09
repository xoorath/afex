#pragma once

// System
#include <cstdint>
#include <cstddef>
#include <new>
#include <type_traits>

//////////////////////////////////////////////////////////////////////////
namespace Core
{
    //////////////////////////////////////////////////////////////////////////
    // This class template provides a way to encapsulate the implementation 
    // details of a class in a private implementation (PIMPL) to reduce 
    // compilation dependencies and improve encapsulation. The implementation
    // object is stored within a fixed-size buffer inside the class to avoid
    // dynamic memory allocation and improve cache locality.
    //
    // A couple gotchas:
    //  1. Be aware k_BufferSize has to be a hard-coded sizeof(T). That may
    //      change based on platform or preprocessor macros. To avoid 
    //      headaches try to refrain from adding conditional members to your
    //      implementation types.
    //  2. Constructors and destructors of the owning class must be implemented
    //      in their cpp file with T included. You can still use '=default'
    //      so long as that is implemented in the cpp file.
    //  3. The type T can be constructed with the same constructor argument 
    //      types as T supports. However: the default constructor of PIMPL never
    //      uses the default constructor of type T and instead creates an
    //      uninitialized PIMPL object. You can later construct with default
    //      arguments (or any arguments) using the emplace function.
    template<typename T, size_t k_BufferSize>
    class PIMPL
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // Valid with T forward declared:

        constexpr size_t GetSize() const noexcept
        {
            return k_BufferSize;
        }

        //////////////////////////////////////////////////////////////////////////
        // Requires T to be included:

        explicit PIMPL()
            : m_Buffer{ 0 }
            , m_Constructed(false)
        {
            static_assert(k_BufferSize >= sizeof(T), "Buffer size too small for type T.");
            static_assert(static_cast<int64_t>(k_BufferSize)-static_cast<int64_t>(sizeof(T)) < 128, 
                "Buffer size is excessively large for type T.");
        }

        template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) != 0)>>
        PIMPL(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
            : m_Constructed(false)
        {
            static_assert(k_BufferSize >= sizeof(T), "Buffer size too small for type T.");
            static_assert(static_cast<int64_t>(k_BufferSize) - static_cast<int64_t>(sizeof(T)) < 128,
                "Buffer size is excessively large for type T.");
            new(getAlignedBuffer()) T(std::forward<Args>(args)...);
            m_Constructed = true;
        }

        template<typename... Args>
        T* emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
        {
            reset();
            new(getAlignedBuffer()) T(std::forward<Args>(args)...);
            m_Constructed = true;
            return GetMutable();
        }

        const T* Get() const noexcept
        {
            return reinterpret_cast<const T*>(getAlignedBuffer());
        }

        T* GetMutable() noexcept
        {
            return reinterpret_cast<T*>(getAlignedBuffer());
        }

        T* operator->() noexcept
        {
            return GetMutable();
        }

        const T* operator->() const noexcept
        {
            return Get();
        }

        void reset()
        {
            if(m_Constructed)
            {
                GetMutable()->~T();
                m_Constructed = false;
            }
        }

        template<typename U = T, typename = std::enable_if_t<std::is_copy_constructible<U>::value>>
        PIMPL(const PIMPL<T, k_BufferSize>& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
        {
            if (other.m_Constructed)
            {
                new(getAlignedBuffer()) T(*other.Get());
                m_Constructed = true;
            }
            else
            {
                m_Constructed = false;
            }
        }

        template<typename U = T, typename = std::enable_if_t<std::is_move_constructible<U>::value>>
        PIMPL(PIMPL<T, k_BufferSize>&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
        {
            if (other.m_Constructed)
            {
                new(getAlignedBuffer()) T(std::move(*other.GetMutable()));
                m_Constructed = true;
                other.reset();
            }
            else
            {
                m_Constructed = false;
            }
        }

        template<typename U = T, typename = std::enable_if_t<std::is_copy_assignable<U>::value>>
        PIMPL& operator=(const PIMPL<T, k_BufferSize>& other) noexcept(std::is_nothrow_copy_assignable<T>::value)
        {
            if (this != &other)
            {
                if (other.m_Constructed)
                {
                    if (m_Constructed)
                    {
                        *GetMutable() = *other.Get();
                    }
                    else
                    {
                        new(getAlignedBuffer()) T(*other.Get());
                        m_Constructed = true;
                    }
                }
                else
                {
                    reset();
                }
            }
            return *this;
        }

        template<typename U = T, typename = std::enable_if_t<std::is_move_assignable<U>::value>>
        PIMPL& operator=(PIMPL<T, k_BufferSize>&& other) noexcept(std::is_nothrow_move_assignable<T>::value)
        {
            if (this != &other)
            {
                if (other.m_Constructed)
                {
                    if (m_Constructed)
                    {
                        *GetMutable() = std::move(*other.GetMutable());
                    }
                    else
                    {
                        new(getAlignedBuffer()) T(std::move(*other.GetMutable()));
                        m_Constructed = true;
                    }
                    other.reset();
                }
                else
                {
                    reset();
                }
            }
            return *this;
        }

        ~PIMPL()
        {
            reset();
        }

    private:
        // Adjust the buffer size to accommodate alignment
        alignas(std::max_align_t) uint8_t m_Buffer[k_BufferSize + alignof(std::max_align_t)];
        bool m_Constructed;

        void* getAlignedBuffer() noexcept
        {
            // Align the buffer manually
            uintptr_t bufferAddress = reinterpret_cast<uintptr_t>(m_Buffer);
            uintptr_t alignedAddress = (bufferAddress + alignof(T) - 1) & ~(alignof(T) - 1);
            return reinterpret_cast<void*>(alignedAddress);
        }

        const void* getAlignedBuffer() const noexcept
        {
            uintptr_t bufferAddress = reinterpret_cast<uintptr_t>(m_Buffer);
            uintptr_t alignedAddress = (bufferAddress + alignof(T) - 1) & ~(alignof(T) - 1);
            return reinterpret_cast<const void*>(alignedAddress);
        }
    };
}