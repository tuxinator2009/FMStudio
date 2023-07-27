#pragma once

const FMSynth::Patch patch_dist_guitar =
{
  .name="DIST.GUITAR", 
  .algorithm=8, .volume=80, .feedback=32, .glide=0, .attack=0, .decay=0, .sustain=100, .release=45, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=71, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false},
    {.level=85, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=51, .attack=0, .decay=60, .sustain=70, .loop=false},
    {.level=71, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false},
    {.level=75, .pitch={.fixed=false, .coarse=1, .fine=50}, .detune=51, .attack=0, .decay=65, .sustain=60, .loop=false}
  }
};
