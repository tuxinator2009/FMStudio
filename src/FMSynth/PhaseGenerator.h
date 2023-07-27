
#pragma once

#include <cstdint>
#include "FixedPoint/Fixed.h"
#include "FixedPoint/FixedMath.h"

namespace FMSynth {

class PhaseGenerator {
    
    public:
    
        constexpr PhaseGenerator(): _phase_Q32(0), _rate_Q32(0) {}
        
        ~PhaseGenerator() = default;
        PhaseGenerator(const PhaseGenerator&) = delete;
        PhaseGenerator& operator =(const PhaseGenerator&) = delete;
        
        inline std::int32_t tick() {
            _phase_Q32 += _rate_Q32;
            return _phase_Q32 >> (32 - 15);
        }
        
        // pm_Q15: Phase modulation as Q15 fixed point
        // Returns phase 0...32767
        inline std::int32_t tick(std::int32_t pm_Q15) {
            _phase_Q32 += _rate_Q32;
            return (_phase_Q32 + (pm_Q15 << (32 - 15))) >> (32 - 15);
        }
        
        // rate_Q32 is frequency as a fraction of sample rate
        inline void setRate(std::uint32_t rate_Q32) {
            _rate_Q32 = rate_Q32;
        }
        
        inline void trigger() {
            _phase_Q32 = 0;
        }
        
        inline void trigger(std::int32_t phase_Q15) {
            _phase_Q32 = phase_Q15 << (32 - 15);
        }
    
    private:
    
        std::uint32_t _phase_Q32;
        std::uint32_t _rate_Q32;
};

} // namespace FMSynth
