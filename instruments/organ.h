#pragma once

const FMSynth::Patch patch_organ =
{
  .name="ORGAN", 
  .algorithm=8, .volume=80, .feedback=50, .glide=0, .attack=0, .decay=0, .sustain=100, .release=50, 
  .lfo={.speed=35, .attack=65, .pmd=3}, 
  .op=
  {
    {.level=77, .pitch={.fixed=false, .coarse=0, .fine=25}, .detune=50, .attack=20, .decay=0, .sustain=100, .loop=false}, 
    {.level=28, .pitch={.fixed=false, .coarse=0, .fine=25}, .detune=57, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=60, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=30, .decay=0, .sustain=100, .loop=false}, 
    {.level=20, .pitch={.fixed=false, .coarse=4, .fine=0}, .detune=50, .attack=45, .decay=0, .sustain=100, .loop=false}
  }
};
