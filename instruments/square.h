#pragma once

const FMSynth::Patch patch_square =
{
  .name="SQUARE", 
  .algorithm=10, .volume=80, .feedback=50, .glide=0, .attack=20, .decay=0, .sustain=100, .release=40, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=0, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=0, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=100, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=25, .pitch={.fixed=false, .coarse=2, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}
  }
};
