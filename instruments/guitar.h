#pragma once

const FMSynth::Patch patch_guitar =
{
  .name="GUITAR", 
  .algorithm=6, .volume=80, .feedback=50, .glide=0, .attack=0, .decay=70, .sustain=0, .release=60, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=100, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=90, .pitch={.fixed=false, .coarse=2, .fine=0}, .detune=50, .attack=8, .decay=55, .sustain=0, .loop=false}, 
    {.level=25, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=50, .sustain=85, .loop=false}, 
    {.level=53, .pitch={.fixed=false, .coarse=5, .fine=0}, .detune=50, .attack=0, .decay=40, .sustain=0, .loop=false}
  }
};
