
#pragma once

#include <cstdint>
#include "FixedPoint/Fixed.h"
#include "FixedPoint/FixedMath.h"

namespace FMSynth {

class EnvelopeGenerator {
    
    public:
    
        enum Stage: std::uint8_t { Attack, Decay, Sustain, Release, Idle };
        
        static constexpr const std::uint32_t LVL_Q = 10;
        static constexpr const std::uint32_t RATE_Q = 20;
        
        constexpr EnvelopeGenerator():
            _ramp_Q20(0), _levels_Q10{1<<LVL_Q, 1<<LVL_Q, 1<<LVL_Q, 0, 0}, _rates_Q20{1<<RATE_Q, 1<<RATE_Q, 0, 1<<RATE_Q, 0},
            _level_a_Q10(0), _level_b_Q10(0), _delta_Q10(0), _rate_Q20(0), _stage(Stage::Idle), _loop_begin(0), _loop_end(0)
        {}
        
        ~EnvelopeGenerator() = default;
        EnvelopeGenerator(const EnvelopeGenerator&) = delete;
        EnvelopeGenerator& operator =(const EnvelopeGenerator&) = delete;
        
        inline std::int32_t tick() {
            _ramp_Q20 += _rate_Q20;
            if(_ramp_Q20 < (1 << RATE_Q)) {
                // Ramp hasn't reached it's upper limit, return interpolated level based on current value of the ramp (0...1 as fixed point)
                return _level_a_Q10 + ((_ramp_Q20 * _delta_Q10) >> RATE_Q);
            }
            
            _setStage((_stage + 1 != _loop_end) ? (_stage + 1) : _loop_begin, _level_b_Q10);
            
            return _level_a_Q10;
        }
        
        inline void trigger() {
            // Slope always starts from zero level
            _setStage(Stage::Attack, 0);
        }
        
        inline void release() {
            if(_stage < Stage::Release) {
                // Calculate current level by interpolation and use it as starting level
                _setStage(Stage::Release, _level_a_Q10 + ((_ramp_Q20 * _delta_Q10) >> RATE_Q));
            }
        }
        
        inline void setAttackRate(const FixedPoint::Fixed<RATE_Q>& rate) {
            _rates_Q20[0] = rate.internal();
        }
        
        inline void setDecayRate(const FixedPoint::Fixed<RATE_Q>& rate) {
            _rates_Q20[1] = rate.internal();
        }
        
        inline void setReleaseRate(const FixedPoint::Fixed<RATE_Q>& rate) {
            _rates_Q20[3] = rate.internal();
        }
        
        inline void setSustain(const FixedPoint::Fixed<LVL_Q>& level) {
            _levels_Q10[1] = level.internal();
        }
        
        inline void setLoop(std::uint8_t begin, std::uint8_t end) {
            _loop_begin = begin;
            _loop_end = end;
        }
        
        inline std::uint8_t stage() const { return _stage; }
    
    private:
    
        inline void _setStage(std::uint8_t stage, std::int32_t start_level) {
            _stage = stage;
            
            _level_a_Q10 = start_level;
            
            // Get slope end level for the next stage and calculate difference
            _level_b_Q10 = _levels_Q10[_stage];
            _delta_Q10 = _level_b_Q10 - _level_a_Q10;
            
            // Get rate for the next stage and reset ramp to zero
            _rate_Q20 = _rates_Q20[_stage];
            _ramp_Q20 = 0;
        }
        
        std::int32_t _ramp_Q20;
        
        std::int32_t _levels_Q10[5];
        std::int32_t _rates_Q20[5];
        
        std::int32_t _level_a_Q10;
        std::int32_t _level_b_Q10;
        std::int32_t _delta_Q10;
        std::int32_t _rate_Q20;
        
        std::uint8_t _stage;
        std::uint8_t _loop_begin;
        std::uint8_t _loop_end;
};

} // namespace FMSynth
