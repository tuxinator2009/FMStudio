#pragma once

FMSynth::Patch patch_Trumpet2 =
{
  .name="Trumpet 2",
  .algorithm=9, .volume=80, .feedback=73, .glide=0, .attack=22, .decay=0, .sustain=100, .release=25,
  .lfo={.speed=0, .attack=0, .pmd=0},
  .op=
  {
    {.level=50, .pitch={.fixed=false, .coarse=2, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false},
    {.level=50, .pitch={.fixed=true, .coarse=0, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false},
    {.level=23, .pitch={.fixed=false, .coarse=1, .fine=49}, .detune=47, .attack=0, .decay=0, .sustain=100, .loop=false},
    {.level=36, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}
  }
};
