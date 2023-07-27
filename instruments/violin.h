#pragma once

const FMSynth::Patch patch_violin =
{
  .name="VIOLIN", 
  .algorithm=6, .volume=80, .feedback=50, .glide=0, .attack=40, .decay=0, .sustain=100, .release=40, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=71, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=53, .attack=50, .decay=60, .sustain=100, .loop=false},
    {.level=71, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=40, .decay=70, .sustain=80, .loop=false},
    {.level=40, .pitch={.fixed=false, .coarse=3, .fine=0}, .detune=50, .attack=25, .decay=0, .sustain=100, .loop=false},
    {.level=30, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=51, .attack=25, .decay=65, .sustain=70, .loop=false}
  }
};
