#pragma once

const FMSynth::Patch patch_gong =
{
  .name="GONG", 
  .algorithm=7, .volume=80, .feedback=50, .glide=0, .attack=0, .decay=85, .sustain=0, .release=50, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=100, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=50, .attack=0, .decay=85, .sustain=0, .loop=false}, 
    {.level=28, .pitch={.fixed=false, .coarse=0, .fine=80}, .detune=50, .attack=0, .decay=85, .sustain=0, .loop=false}, 
    {.level=28, .pitch={.fixed=false, .coarse=0, .fine=75}, .detune=50, .attack=75, .decay=80, .sustain=25, .loop=false}, 
    {.level=40, .pitch={.fixed=false, .coarse=1, .fine=40}, .detune=50, .attack=75, .decay=0, .sustain=100, .loop=false}
  }
};
