#pragma once

const FMSynth::Patch patch_bass =
{
  .name="BASS", 
  .algorithm=8, .volume=80, .feedback=50, .glide=0, .attack=0, .decay=70, .sustain=0, .release=35, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=100, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=50, .attack=0, .decay=70, .sustain=0, .loop=false}, 
    {.level=62, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=54, .attack=25, .decay=55, .sustain=0, .loop=false}, 
    {.level=100, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=50, .attack=0, .decay=70, .sustain=0, .loop=false}, 
    {.level=20, .pitch={.fixed=false, .coarse=1, .fine=50}, .detune=50, .attack=0, .decay=60, .sustain=0, .loop=false}
  }
};
