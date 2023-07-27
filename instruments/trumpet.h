#pragma once

const FMSynth::Patch patch_trumpet =
{
  .name="TRUMPET", 
  .algorithm=8, .volume=80, .feedback=73, .glide=0, .attack=30, .decay=0, .sustain=100, .release=40, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=71, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=52, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=55, .attack=40, .decay=50, .sustain=75, .loop=false}, 
    {.level=71, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=57, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=44, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=53, .attack=35, .decay=50, .sustain=75, .loop=false}
  }
};
