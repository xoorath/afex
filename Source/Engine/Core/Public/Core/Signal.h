#pragma once

// Engine
#include <Core/Core.export.h>

// System
#include <functional>
#include <list>
#include <mutex>

namespace Core
{
    template<typename T>
    class Signal
    {
    public:
        using Delegate = std::function<T>;
        using FuncList = std::list<Delegate>;
        using Token = typename std::list<Delegate>::iterator;

        inline  Signal::Token Add(Signal::Delegate del)
        {
            // todo: replace all lock_guards here with scoped_lock when we turn on c++ >= 17
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Funcs.push_back(del);
            return --m_Funcs.end();
        }

        inline Signal::Token operator += (Signal::Delegate del)
        {
            return Add(del);
        }

        inline bool Remove(Signal::Token token)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_Funcs.end() == token)
            {
                return false;
            }
            m_Funcs.erase(token);
            return true;
        }

        inline bool operator -= (Signal::Token token)
        {
            return Remove(token);
        }

        template <typename... Args> 
        inline void Invoke(Args &&...args) const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            for (auto const& func : m_Funcs)
            {
                func(std::forward<Args>(args)...);
            }
        }

        template <typename... Args>
        inline void operator()(Args &&...args) const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            for (auto const& func : m_Funcs)
            {
                func(std::forward<Args>(args)...);
            }
        }

        operator bool() const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Funcs.size() > 0;
        }

        Signal()                            = default;
        Signal(const Signal&)               = delete;
        Signal& operator=(const Signal&)    = delete;
        ~Signal()                           = default;

        Signal(Signal&& other) noexcept
        {
            std::lock_guard<std::mutex> lock(other.m_Mutex);
            m_Funcs = std::move(other.m_Funcs);
        }

        Signal& operator=(Signal&& other) noexcept
        {
            std::lock_guard<std::mutex> lock(other.m_Mutex);
            m_Funcs = std::move(other.m_Funcs);
            return *this;
        }

    private:
        mutable std::mutex m_Mutex;
        FuncList m_Funcs;
    };
}