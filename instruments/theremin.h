#pragma once

const FMSynth::Patch patch_theremin =
{
  .name="THEREMIN", 
  .algorithm=1, .volume=80, .feedback=50, .glide=45, .attack=40, .decay=0, .sustain=100, .release=60, 
  .lfo={.speed=70, .attack=60, .pmd=5}, 
  .op=
  {
    {.level=100, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=20, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=51, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=0, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=0, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}
  }
};
