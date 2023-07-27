#pragma once

const FMSynth::Patch patch_celesta =
{
  .name="CELESTA", 
  .algorithm=5, .volume=80, .feedback=50, .glide=0, .attack=5, .decay=70, .sustain=0, .release=55, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=100, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=70, .sustain=0, .loop=false}, 
    {.level=5, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=70, .attack=0, .decay=50, .sustain=0, .loop=false}, 
    {.level=20, .pitch={.fixed=false, .coarse=12, .fine=0}, .detune=50, .attack=0, .decay=15, .sustain=0, .loop=false}, 
    {.level=0, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}
  }
};
