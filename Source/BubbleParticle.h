#pragma once

//==============================================================================
// Single bubble particle for the beer glass animation
//==============================================================================
struct BubbleParticle
{
    float x           { 0.0f };   // horizontal position (pixels, within glass)
    float y           { 0.0f };   // vertical position   (pixels, from bottom)
    float radius      { 3.0f };   // bubble radius in pixels
    float speed       { 20.0f };  // rise speed in pixels/sec
    float wobble      { 4.0f };   // horizontal wobble amplitude in pixels
    float wobblePhase { 0.0f };   // current wobble phase (radians)
    float wobbleFreq  { 2.0f };   // wobble frequency (radians/sec)
    float alpha       { 0.7f };   // opacity 0..1
    float lifetime    { 1.0f };   // remaining life 0..1 (1 = just spawned, 0 = dead)
};
