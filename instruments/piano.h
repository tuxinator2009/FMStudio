#pragma once

const FMSynth::Patch patch_piano =
{
  .name="PIANO",
  .algorithm=3, .volume=80, .feedback=50, .glide=0, .attack=0, .decay=75, .sustain=100, .release=60,
  .lfo={.speed=0, .attack=0, .pmd=0},
  .op=
  {
    {.level=71, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=70, .sustain=0, .loop=false}, 
    {.level=23, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=57, .attack=0, .decay=70, .sustain=0, .loop=false}, 
    {.level=20, .pitch={.fixed=false, .coarse=5, .fine=0}, .detune=51, .attack=0, .decay=70, .sustain=0, .loop=false}, 
    {.level=40, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=50, .sustain=0, .loop=false}
  }
};
