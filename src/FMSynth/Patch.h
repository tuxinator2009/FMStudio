#pragma once

#include <cstdint>

namespace FMSynth {

struct Patch {
    char name[16];
    std::int8_t algorithm; //1 - 11
    std::int8_t volume;//0 - 100
    std::int8_t feedback;//-50 - 50 (+50)
    std::int8_t glide;//0 - 100
    
    std::int8_t attack;//0 - 100
    std::int8_t decay;//0 - 100
    std::int8_t sustain;//0 - 100
    std::int8_t release;//0 - 100
    
    struct LFO {
        std::int8_t speed;//0 - 100
        std::int8_t attack;//0 - 100
        std::int8_t pmd;//0 - 100 (Depth)
    } lfo;
    
    struct Operator {
        std::uint8_t level;//0 - 100
        
        struct Pitch {
            bool fixed;
            std::int8_t coarse;//0 - 15
            std::int8_t fine;//0 - 99
        } pitch;
        
        std::int8_t detune;//0 - 100
        
        std::int8_t attack;//0 - 100
        std::int8_t decay;//0 - 100
        std::int8_t sustain;//0 - 100
        bool loop;
    } op[4];
};

} // namespace FMSynth
