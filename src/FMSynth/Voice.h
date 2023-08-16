
#pragma once

#include "FixedPoint/Fixed.h"
#include "PhaseGenerator.h"
#include "EnvelopeGenerator.h"
#include "Patch.h"

namespace FMSynth {

template<unsigned Samplerate>
class Voice {
    
    public:
    
        Voice(): _volume_Q10(0), _fb_level_Q10(0), _feedback_Q15(0), _pitchbend_Q15(0), _cur_algo(_null_algorithm) {}
        
        ~Voice() = default;
        
        Voice(const Voice&) = delete;
        Voice& operator =(const Voice&) = delete;
        
        inline std::uint8_t update() {
            std::int32_t out = 128 + _cur_algo(this) / (1<<(16-8)); // convert signed 16 bit to unsigned 8 bit
            return out > 255 ? 255 : (out < 0 ? 0 : out);
        }
        
        void noteOn(const Patch& patch, std::int8_t midikey, std::int8_t velocity) {
            _initOperatorRatesAndLevels(midikey, patch);
            
            if(patch.glide > 0 && _master_env_gen.stage() < EnvelopeGenerator::Stage::Release) {
                // First calculate interval from previous midikey to current glide position
                _glide_interval_Q10 = (_glide_interval_Q10 * (_glide_level_Q20 >> 5)) / (1 << 15);
                // Then add interval from current to previous midikey
                _glide_interval_Q10 += (_midikey - midikey) * (1 << 10) / 12;
                
                FixedPoint::Fixed<20> att_rate(_durationToRate(_percentToDuration(patch.glide)));
                _glide_att_rate_Q20 = att_rate.internal();
                _glide_level_Q20 = 1 << 20; // Envelope starts from max glide interval and glides down to zero to current note
                
                _startLFO(patch.lfo.speed, patch.lfo.attack, patch.lfo.pmd);
            }
            else {
                // Prevent the running algorithm from calling updateControlValues while we are initializing values
                _control_ticks = 0;
                
                _glide_interval_Q10 = 0;
                _glide_att_rate_Q20 = 0;
                _glide_level_Q20 = 0;
                
                for(std::uint32_t idx = 0; idx < 4; ++idx) {
                    _op_gains_Q10[idx] = 0;
                    
                    const Patch::Operator& op = patch.op[idx];
                    _startOperatorEG(idx, op.attack, op.decay, op.sustain, op.loop);
                    _phase_gens[idx].trigger();
                }
                
                _startMasterEG(patch.attack, patch.decay, patch.sustain, patch.release);
                
                _startLFO(patch.lfo.speed, patch.lfo.attack, patch.lfo.pmd);
                
                FixedPoint::Fixed<10> level(_percentToLevel(patch.volume));
                _volume_Q10 = (level.internal() * (velocity < 0 ? 0 : velocity)) >> 7;  // velocity ranges from 0 to 127
                
                level = _percentToLevel(patch.feedback - 50);
                _fb_level_Q10 = level.internal();
                _feedback_Q15 = 0;
                
                std::int32_t algo_idx = patch.algorithm - 1;
                _cur_algo = _algorithms[algo_idx <= 0 ? 0 : (algo_idx >= 10 ? 10 : algo_idx)];
                
                // Ensure updateControlValues is called first time the algorithm is processed. 
                _control_ticks = Samplerate / _CONTROLRATE - 1;
            }
            
            _midikey = midikey;
        }
        
        inline void noteOff() { _master_env_gen.release(); }
        
        inline void setPitchBend(const FixedPoint::Fixed<15>& ratio) {
            constexpr std::int32_t PITCH_BEND_RANGE_Q15 = (1<<15) * 2.0 / 12;
            _pitchbend_Q15 = ratio.internal() * PITCH_BEND_RANGE_Q15 >> 15;
        }
        
        inline std::int8_t midikey() const { return _midikey; }
        
        inline bool released() const { return _master_env_gen.stage() >= EnvelopeGenerator::Stage::Release; }
        inline bool finished() const { return _master_env_gen.stage() == EnvelopeGenerator::Stage::Idle; }
        
        // Returns pitch ratio as Q15 fixed point. interval can be from -108 to 107 semitones (+/- 9 octaves).
        static constexpr std::int32_t semitonesToRatio(std::int32_t semitones) {
            std::int32_t octaves = (108 + semitones) / 12;
            return (static_cast<std::int32_t>(_SEMITONES_Q15[semitones - 12 * (octaves - 9)]) << octaves) >> 8;
        }
        
        // Input range is -9...8.999 as Q15 fixed point
        static constexpr std::uint32_t pow2(std::int32_t exp_Q15) {
            std::uint32_t semitone_Q15 = 12 * (exp_Q15 & 0x7fff);
            std::uint32_t semitone = semitone_Q15 >> 15;
            std::uint32_t ratio_a_Q15 = _SEMITONES_Q15[semitone];
            std::uint32_t ratio_b_Q15 = _SEMITONES_Q15[semitone + 1];
            std::uint32_t fraction_Q15 = semitone_Q15 & 0x7fff;
            
            std::uint32_t shift = 9 + (exp_Q15 >> 15);
            return ((ratio_a_Q15 + (((ratio_b_Q15 - ratio_a_Q15) * fraction_Q15) >> 15)) << shift) >> 8;
        }
        
        static constexpr FixedPoint::Fixed<15> toFixedFrequency(std::int8_t coarse, std::int8_t fine) {
            constexpr std::int32_t base_Q15 = (1<<15) * 4096 / 100.0;
            std::int32_t freq_Q15(coarse > 0 ? (((100 + fine) * base_Q15) >> (15 - coarse)) : ((fine * base_Q15) >> (15 - 1)));
            return FixedPoint::Fixed<15>::fromInternal(freq_Q15) + FixedPoint::Fixed<15>(0.00099999);  // For rounding up to next 1/1000
        }
        
        static constexpr FixedPoint::Fixed<15> toLFOFrequency(std::int8_t speed) {
            constexpr std::int32_t SCALE_Q15 = (1<<15) * 3.32193 * 0.02 + 0.5;
            constexpr std::int32_t MIN_Q10 = (1<<10) * 0.2 + 0.5;
            return FixedPoint::Fixed<15>::fromInternal((MIN_Q10 * pow2(speed * SCALE_Q15)) >> 10);
        }
    
    private:
    
        inline void _initOperatorRatesAndLevels(std::int8_t midikey, const Patch& patch) {
            std::int32_t pitch_Q15 = 440 * semitonesToRatio(midikey - 69);
            
            for(std::uint32_t idx = 0; idx < 4; ++idx) {
                const Patch::Operator& op = patch.op[idx];
                
                FixedPoint::Fixed<10> op_level(_percentToLevel(op.level));
                _op_levels_Q10[idx] = op_level.internal();
                
                std::int32_t freq_Q15;
                if(op.pitch.fixed) {
                    freq_Q15 = toFixedFrequency(op.pitch.coarse, op.pitch.fine).internal();
                }
                else {
                    freq_Q15 = (pitch_Q15 * static_cast<uint64_t>(100 * op.pitch.coarse + op.pitch.fine) + 50) / 100;
                }
                _op_fixed[idx] = op.pitch.fixed;
                
                std::uint64_t rate_Q32 = (static_cast<std::uint64_t>(_RATE_1HZ_Q32) * freq_Q15) >> 15;
                _op_rates_Q32[idx] = (rate_Q32 * pow2((((op.detune - 50) << 15) + 600) / 1200)) >> 15;
            }
        }
        
        inline void _startLFO(std::uint8_t speed, std::int8_t delay, std::int8_t pmd) {
            // Set LFO rate
            std::uint32_t freq_Q15 = toLFOFrequency(speed).internal();
            _lfo_phase_gen.setRate((freq_Q15 / _CONTROLRATE) << (32 - 15));
            
            // Set LFO amplitude
            FixedPoint::Fixed<10> depth(_percentToLevel(pmd));
            _lfo_depth_Q10 = depth.internal() / 2;
            
            // Set LFO envelope attack rate
            FixedPoint::Fixed<20> att_rate(_durationToRate(_percentToDuration(delay)));
            _lfo_att_rate_Q20 = att_rate.internal();
            
            _lfo_level_Q20 = 0;
            
            _lfo_phase_gen.trigger(0.25 * (1<<15));  // Set initial phase so that triangle wave starts from zero
        }
        
        inline void _startOperatorEG(std::uint32_t idx, std::int8_t attack, std::int8_t decay, std::int8_t sustain, bool loop) {
            _env_gens[idx].setAttackRate(_durationToRate(_percentToDuration(attack)));
            _env_gens[idx].setDecayRate(_durationToRate(_percentToDuration(decay)));
            _env_gens[idx].setSustain(FixedPoint::Fixed<10>(sustain) / 100);
            _env_gens[idx].setLoop(0, loop ? 2 : 0);
            _env_gens[idx].trigger();
        }
        
        inline void _startMasterEG(std::int8_t attack, std::int8_t decay, std::int8_t sustain, std::int8_t release) {
            _master_env_gen.setAttackRate(_durationToRate(_percentToDuration(attack)));
            _master_env_gen.setDecayRate(_durationToRate(_percentToDuration(decay)));
            _master_env_gen.setSustain(FixedPoint::Fixed<10>(sustain) / 100);
            _master_env_gen.setReleaseRate(_durationToRate(_percentToDuration(release)));
            _master_env_gen.trigger();
        }
        
        void _updateControlValues() {
            // LFO envelope
            if(_lfo_level_Q20 < (1<<20)) {
                _lfo_level_Q20 += _lfo_att_rate_Q20;
                if(_lfo_level_Q20 >= (1<<20)) _lfo_level_Q20 = (1<<20);
            }
            std::int32_t lfo_gain_Q10 = (_lfo_depth_Q10 * _lfo_level_Q20) >> 20;
            std::int32_t lfo_pitch_Q15 = (lfo_gain_Q10 * _tri(_lfo_phase_gen.tick())) / (1<<10);
            
            // Pitch ratio for fixed frequency operators
            std::int32_t pitch_ratio_fixed_Q10 = pow2(lfo_pitch_Q15 + _pitchbend_Q15) >> 5;
            
            // Glide envelope
            if(_glide_level_Q20 > 0) {
                _glide_level_Q20 -= _glide_att_rate_Q20;
                if(_glide_level_Q20 <= 0) _glide_level_Q20 = 0;
            }
            std::int32_t pitchglide_Q15 = (_glide_interval_Q10 * (_glide_level_Q20 >> 5)) / (1<<10);
            
            // Pitch ratio including glide
            std::int32_t pitch_ratio_glide_Q10 = pow2(lfo_pitch_Q15 + _pitchbend_Q15 + pitchglide_Q15) >> 5;
            
            _master_gain_Q10 = (_volume_Q10 * _master_env_gen.tick()) >> 10;
            _master_gain_Q10 = (_master_gain_Q10 * _master_gain_Q10) >> 10;
            
            std::int32_t env_level_Q10;
            for(std::uint32_t idx = 0; idx < 4; ++idx) {
                _phase_gens[idx].setRate((_op_rates_Q32[idx] >> 10) * (_op_fixed[idx] ? pitch_ratio_fixed_Q10 : pitch_ratio_glide_Q10));
                
                // Get current level of operator's enevelope generator
                env_level_Q10 = _env_gens[idx].tick();
                
                std::int32_t op_level_Q15 = (_op_levels_Q10[idx] * env_level_Q10) >> 5;
                _op_gains_Q10[idx] = (op_level_Q15 * op_level_Q15) >> 20;
            }
            
            // Set operator 4 feedback gain, feedback is affected by operator's envelope generator
            _fb_gain_Q10 = (((_fb_level_Q10 * _fb_level_Q10) >> 5) * ((env_level_Q10 * env_level_Q10) >> 5)) >> 20;
            _fb_gain_Q10 = _fb_level_Q10 < 0 ? -_fb_gain_Q10 : _fb_gain_Q10;
        }
        
        // Quadratic function sine approximation (16 asm lines). Max error 2.8 % at angles 27, 153, 207 and 333 deg.
        static constexpr std::int32_t _sin(std::int32_t phase_Q15) {
            std::int32_t p = phase_Q15 & 16383;
            return (phase_Q15 & 16384 ? (p - 16384) : (16384 - p)) * p / 2048;
        }
        
        static constexpr std::int32_t _sqr(std::int32_t phase_Q15) {
            return (phase_Q15 & 16384) ? -32768 : 32768;
        }
        
        static constexpr std::int32_t _saw(std::int32_t phase_Q15) {
            return 2 * (phase_Q15 & 32767) - 32767;
        }
        
        static constexpr std::int32_t _tri(std::int32_t phase_Q15) {
            std::int32_t p = 4 * (phase_Q15 & 16383);
            return (phase_Q15 & 16384) ? 32767 - p : p - 32768;
        }
        
        // Returns level as Q10 fixed point
        static constexpr FixedPoint::Fixed<10> _percentToLevel(std::int32_t level) {
            constexpr std::int32_t SCALE_Q24 = (1<<30) * 0.01;
            return FixedPoint::Fixed<10>::fromInternal((SCALE_Q24 * level + (1<<19)) >> 20);
        }
        
        // Takes percent (0 to 100) and returns duration in seconds (5 ms to 50 s, or 0 when input is zero) as Q20 fixed point
        static constexpr FixedPoint::Fixed<20> _percentToDuration(std::int8_t percent) {
            constexpr std::int32_t SCALE_Q15 = (1<<15) * 0.04 * 3.32193;  // 0.04 * log2(10)
            return FixedPoint::Fixed<20>::fromInternal(percent > 0 ? (pow2(SCALE_Q15 * (percent - 50)) << (5-1)) : 0);
        }
        
        static constexpr FixedPoint::Fixed<20> _durationToRate(const FixedPoint::Fixed<20>& duration) {
            std::uint32_t div_Q10 = (static_cast<std::uint64_t>(_CONTROLRATE) * duration.internal()) >> 10;
            return FixedPoint::Fixed<20>::fromInternal((div_Q10 <= 1) ? (1<<20) : ((1<<30) / div_Q10));
        }
        
        static constexpr std::int32_t _CONTROLRATE = 500;
        
        // Multiplier to convert values of range 0...100 to range 0...1024 (as Q10 fixed point)
        static constexpr std::int32_t _LEVEL_SCALE = (1<<10) * 10.24;
        static constexpr std::int32_t _RATE_1HZ_Q32 = (static_cast<std::uint64_t>(1) << 32) / Samplerate;
        
        static constexpr std::uint16_t _SEMITONES_Q15[13] = {
            static_cast<std::uint16_t>((1<<15) * 0.50000),
            static_cast<std::uint16_t>((1<<15) * 0.52973),
            static_cast<std::uint16_t>((1<<15) * 0.56123),
            static_cast<std::uint16_t>((1<<15) * 0.59460),
            static_cast<std::uint16_t>((1<<15) * 0.62996),
            static_cast<std::uint16_t>((1<<15) * 0.66742), 
            static_cast<std::uint16_t>((1<<15) * 0.70711),
            static_cast<std::uint16_t>((1<<15) * 0.74915),
            static_cast<std::uint16_t>((1<<15) * 0.79370),
            static_cast<std::uint16_t>((1<<15) * 0.84090),
            static_cast<std::uint16_t>((1<<15) * 0.89090),
            static_cast<std::uint16_t>((1<<15) * 0.94387), 
            static_cast<std::uint16_t>((1<<15) * 1.00000)
        };
        
        PhaseGenerator _phase_gens[4];
        EnvelopeGenerator _env_gens[4];
        
        EnvelopeGenerator _master_env_gen;
        
        // LFO for vibrato effect
        PhaseGenerator _lfo_phase_gen;
        std::int32_t _lfo_depth_Q10;
        
        std::int32_t _lfo_level_Q20;
        std::int32_t _lfo_att_rate_Q20;
        
        // Glide
        std::int32_t _glide_interval_Q10;
        std::int32_t _glide_att_rate_Q20;
        std::int32_t _glide_level_Q20;
        
        std::int8_t _midikey;
        
        // Operator levels and frequencies
        std::int32_t _op_levels_Q10[4];
        std::uint32_t _op_rates_Q32[4];
        bool _op_fixed[4];
        
        std::int32_t _op_gains_Q10[4];
        std::int32_t _master_gain_Q10;
        
        // Master volume
        std::int32_t _volume_Q10;
        
        // Operator 4 feedback
        std::int32_t _fb_level_Q10;
        std::int32_t _fb_gain_Q10;
        std::int32_t _feedback_Q15;
        
        std::int32_t _pitchbend_Q15;
        
        std::uint32_t _control_ticks;
        
        using Algorithm = std::int32_t (*)(void*);
        
        Algorithm _cur_algo;
        
        static std::int32_t _null_algorithm(void*) { return 0; }
        
        template<unsigned Index>
        static std::int32_t algorithm(void* data) {
            Voice& self = *reinterpret_cast<Voice*>(data);
            
            ++self._control_ticks;
            if(self._control_ticks >= Samplerate / _CONTROLRATE) {
                self._control_ticks = 0;
                
                self._updateControlValues();
                
                if(self._master_env_gen.stage() >= EnvelopeGenerator::Stage::Idle) {
                    self._cur_algo = _null_algorithm;
                }
            }
            
            std::int32_t out4 = _sin(self._phase_gens[3].tick(self._feedback_Q15));
            // Feedback value is op4 output (or output^2 if fb is negative) multiplied by 2.25*_fb_gain
            self._feedback_Q15 = (self._fb_gain_Q10 < 0 ? ((out4 * out4) >> 15) : out4) * 9 * self._fb_gain_Q10 / (1 << (10 + 2));
            
            // References to _phase_gens and _op_gains arrays
            PhaseGenerator (&phase_gens)[4] = self._phase_gens;
            std::int32_t (&op_gains_Q10)[4] = self._op_gains_Q10;
            
            // For each implementation of this function template, only one branch is chosen at compile time based on the tamplate parameter
            if(Index == 0) {
                // Modulator outputs are multiplied by 2.25*op_gain
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick(out4)) * 9 * op_gains_Q10[2] / (1 << (10 + 2));
                std::int32_t out2 = _sin(phase_gens[1].tick(out3)) * 9 * op_gains_Q10[1] / (1 << (10 + 2));
                
                // Carrier output is multiplied by just op_gain
                std::int32_t out1 = _sin(phase_gens[0].tick(out2)) * op_gains_Q10[0] / (1 << 10);
                
                // Return carrier outputs multiplied by _master_gain
                return out1 * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 1) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick()) * 9 * op_gains_Q10[2] / (1 << (10 + 2));
                std::int32_t out2 = _sin(phase_gens[1].tick(out3 + out4)) * 9 * op_gains_Q10[1] / (1 << (10 + 2));
                std::int32_t out1 = _sin(phase_gens[0].tick(out2)) * op_gains_Q10[0] / (1 << 10);
                return out1 * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 2) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick()) * 9 * op_gains_Q10[2] / (1 << (10 + 2));
                std::int32_t out2 = _sin(phase_gens[1].tick(out3)) * 9 * op_gains_Q10[1] / (1 << (10 + 2));
                std::int32_t out1 = _sin(phase_gens[0].tick(out2 + out4)) * op_gains_Q10[0] / (1 << 10);
                return out1 * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 3) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick(out4)) * 9 * op_gains_Q10[2] / (1 << (10 + 2));
                std::int32_t out2 = _sin(phase_gens[1].tick(out4)) * 9 * op_gains_Q10[1] / (1 << (10 + 2));
                std::int32_t out1 = _sin(phase_gens[0].tick(out2 + out3)) * op_gains_Q10[0] / (1 << 10);
                return out1 * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 4) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick(out4)) * 9 * op_gains_Q10[2] / (1 << (10 + 2));
                std::int32_t out2 = _sin(phase_gens[1].tick(out3)) * op_gains_Q10[1] / (1 << 10);
                std::int32_t out1 = _sin(phase_gens[0].tick(out3)) * op_gains_Q10[0] / (1 << 10);
                return (out1 + out2) * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 5) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick(out4)) * 9 * op_gains_Q10[2] / (1 << (10 + 2));
                std::int32_t out2 = _sin(phase_gens[1].tick(out3)) * op_gains_Q10[1] / (1 << 10);
                std::int32_t out1 = _sin(phase_gens[0].tick()) * op_gains_Q10[0] / (1 << 10);
                return (out1 + out2) * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 6) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick()) * 9 * op_gains_Q10[2] / (1 << (10 + 2));
                std::int32_t out2 = _sin(phase_gens[1].tick()) * 9 * op_gains_Q10[1] / (1 << (10 + 2));
                std::int32_t out1 = _sin(phase_gens[0].tick(out2 + out3 + out4)) * op_gains_Q10[0] / (1 << 10);
                return out1 * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 7) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick(out4)) * op_gains_Q10[2] / (1 << 10);
                std::int32_t out2 = _sin(phase_gens[1].tick()) * 9 * op_gains_Q10[1] / (1 << (10 + 2));
                std::int32_t out1 = _sin(phase_gens[0].tick(out2)) * op_gains_Q10[0] / (1 << 10);
                return (out1 + out3) * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 8) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick(out4)) * op_gains_Q10[2] / (1 << 10);
                std::int32_t out2 = _sin(phase_gens[1].tick(out4)) * op_gains_Q10[1] / (1 << 10);
                std::int32_t out1 = _sin(phase_gens[0].tick(out4)) * op_gains_Q10[0] / (1 << 10);
                return (out1 + out2 + out3) * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 9) {
                out4 = out4 * 9 * op_gains_Q10[3] / (1 << (10 + 2));
                std::int32_t out3 = _sin(phase_gens[2].tick(out4)) * op_gains_Q10[2] / (1 << 10);
                std::int32_t out2 = _sin(phase_gens[1].tick()) * op_gains_Q10[1] / (1 << 10);
                std::int32_t out1 = _sin(phase_gens[0].tick()) * op_gains_Q10[0] / (1 << 10);
                return (out1 + out2 + out3) * self._master_gain_Q10 / (1 << 10);
            }
            else if(Index == 10) {
                out4 = out4 * op_gains_Q10[3] / (1 << 10);
                std::int32_t out3 = _sin(phase_gens[2].tick()) * op_gains_Q10[2] / (1 << 10);
                std::int32_t out2 = _sin(phase_gens[1].tick()) * op_gains_Q10[1] / (1 << 10);
                std::int32_t out1 = _sin(phase_gens[0].tick()) * op_gains_Q10[0] / (1 << 10);
                return (out1 + out2 + out3 + out4) * self._master_gain_Q10 / (1 << 10);
            }
            
            return 0;
        };
        
        static constexpr Algorithm _algorithms[11] = {
            algorithm<0>, algorithm<1>, algorithm<2>, algorithm<3>, algorithm<4>, algorithm<5>,
            algorithm<6>, algorithm<7>, algorithm<8>, algorithm<9>, algorithm<10>
        };
};

} // namespace FMSynth
