#pragma once

const FMSynth::Patch patch_e_piano =
{
  .name="E.PIANO", 
  .algorithm=8, .volume=80, .feedback=57, .glide=0, .attack=0, .decay=75, .sustain=0, .release=50, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=71, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=70, .sustain=0, .loop=false}, 
    {.level=10, .pitch={.fixed=false, .coarse=14, .fine=0}, .detune=50, .attack=0, .decay=50, .sustain=0, .loop=false}, 
    {.level=71, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=43, .attack=0, .decay=75, .sustain=0, .loop=false}, 
    {.level=35, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=57, .attack=0, .decay=75, .sustain=0, .loop=false}
  }
};
