#pragma once

const FMSynth::Patch patch_cowbell =
{
  .name="COWBELL", 
  .algorithm=8, .volume=80, .feedback=50, .glide=0, .attack=0, .decay=50, .sustain=0, .release=40, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=71, .pitch={.fixed=true, .coarse=13, .fine=74}, .detune=50, .attack=0, .decay=30, .sustain=0, .loop=false}, 
    {.level=25, .pitch={.fixed=true, .coarse=13, .fine=7}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=71, .pitch={.fixed=true, .coarse=11, .fine=87}, .detune=50, .attack=0, .decay=50, .sustain=0, .loop=false}, 
    {.level=13, .pitch={.fixed=true, .coarse=14, .fine=14}, .detune=50, .attack=0, .decay=20, .sustain=25, .loop=false}
  }
}
