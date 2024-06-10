#pragma once

// System
#include <cstdint>

//////////////////////////////////////////////////////////////////////////

namespace bgfx
{
    struct Stats;
}

//////////////////////////////////////////////////////////////////////////
namespace Donsol::UI
{
    //////////////////////////////////////////////////////////////////////////
    class Performance
    {
    public:
        Performance();
        Performance(const Performance&)                     = delete;
        Performance(Performance&&) noexcept                 = delete;
        Performance& operator=(const Performance&)          = delete;
        Performance& operator=(Performance&&) noexcept      = delete;
        ~Performance()                                      = default;
        

        void Update(const bgfx::Stats* stats);
    private:
        void ResetSamples();
        void PushSample(float value);

        // struct to group together sample data
        struct
        {
            static constexpr uint32_t k_NumSamples = 100;
            int32_t m_offset = 0;
            float m_values[k_NumSamples];

            float m_min = 0.0f;
            float m_max = 0.0f;
            float m_avg = 0.0f;
        } m_Samples;
        bool m_showStats = true;
    };
}