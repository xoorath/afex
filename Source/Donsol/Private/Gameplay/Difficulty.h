#pragma once

// System
#include <cstdint>
#include <string>

//////////////////////////////////////////////////////////////////////////
namespace Donsol
{
    //////////////////////////////////////////////////////////////////////////
    enum class DifficultyLevel
    {
        Casual,
        Difficult,
        Metal
    };

    //////////////////////////////////////////////////////////////////////////
    class Difficulty
    {
    public:
        explicit Difficulty(std::string_view name, DifficultyLevel level);

        Difficulty()                                    = delete;
        Difficulty(const Difficulty&)                   = delete;
        Difficulty(Difficulty&&) noexcept               = delete;
        Difficulty& operator=(const Difficulty&)        = delete;
        Difficulty& operator=(Difficulty&&) noexcept    = delete;
        ~Difficulty() = default;

        const std::string_view GetName() const;
        DifficultyLevel GetLevel() const;

        static constexpr size_t GetDifficultyCount() { return sizeof(s_Difficulties) / sizeof(s_Difficulties[0]); }
        static const Difficulty& GetDifficulty(size_t index);

    private:
        std::string m_Name;
        DifficultyLevel m_Level;

        static Difficulty s_Difficulties[3];
    };
}