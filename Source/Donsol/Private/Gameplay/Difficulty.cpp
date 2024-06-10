#include "Difficulty.h"

//////////////////////////////////////////////////////////////////////////
namespace Donsol
{
    ////////////////////////////////////////////////////////////////////////// Public
    /*explicit*/ Difficulty::Difficulty(std::string_view name, DifficultyLevel level)
        : m_Name(name)
        , m_Level(level)
    {
    }

    const std::string_view Difficulty::GetName() const
    {
        return m_Name;
    }

    DifficultyLevel Difficulty::GetLevel() const
    {
        return m_Level;
    }

    /*static*/ const Difficulty& Difficulty::GetDifficulty(size_t index)
    {
        return s_Difficulties[index];
    }

    ////////////////////////////////////////////////////////////////////////// Private
    /*static*/ Difficulty Difficulty::s_Difficulties[3] = 
    {
        Difficulty("Casual",    DifficultyLevel::Casual),
        Difficulty("Difficult", DifficultyLevel::Difficult),
        Difficulty("Metal",     DifficultyLevel::Metal)
    };
}