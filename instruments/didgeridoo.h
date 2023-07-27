#pragma once

const FMSynth::Patch patch_digeridoo_e2 =
{
  .name="DIDGERIDOO E2",
  .algorithm=2, .volume=80, .feedback=50, .glide=40, .attack=35, .decay=0, .sustain=100, .release=55, 
  .lfo={.speed=0, .attack=0, .pmd=0}, 
  .op=
  {
    {.level=100, .pitch={.fixed=true, .coarse=9, .fine=29}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}, 
    {.level=25, .pitch={.fixed=true, .coarse=10, .fine=93}, .detune=50, .attack=0, .decay=55, .sustain=75, .loop=false}, 
    {.level=50, .pitch={.fixed=false, .coarse=0, .fine=50}, .detune=50, .attack=50, .decay=0, .sustain=100, .loop=false}, 
    {.level=0, .pitch={.fixed=false, .coarse=1, .fine=0}, .detune=50, .attack=0, .decay=0, .sustain=100, .loop=false}
  }
};
