#pragma once

const FMSynth::Patch patch_echo =
{
  .name="ECHO", 
  .algorithm=3, .volume=80, .feedback=50, .glide=0, .attack=0, .decay=80, .sustain=0, .release=50, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=100, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=45, .sustain=0, .loop=true}, 
    {.level=0, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=0, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=30, .pitch={.fixed=false, .coarse=2, .fine=0}, .detune=50, .attack=0, .decay=80, .sustain=0, .loop=false}
  }
};
