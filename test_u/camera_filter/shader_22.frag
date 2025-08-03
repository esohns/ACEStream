#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

// horizontal edge detection:
// [  1  2  1 ]
// [  0  0  0 ]
// [ -1 -2 -1 ]
//
// vertical edge detection:
// [  1  0  -1 ]
// [  2  0  -2 ]
// [  1  0  -1 ]
//
// all
// [ 0 -1 0 ]
// [ -1 4 -1 ]
// [ 0 -1 0 ]
//
// sharpen
// [ -1 -1 -1 ]
// [ -1 9 -1 ]
// [ -1 -1 -1 ]

// set the convolution filter 0(none) - 5(blur)
#define FILTER 5

// adjust the distance used in the filter convolution
#define STEP .005

#define BOT 1.-STEP
#define TOP 1.+STEP
#define CEN 1

void
main ()
{
  vec2 uv = gl_FragCoord.xy/iResolution;
    
  gl_FragColor = 
#if FILTER==0
    // none
    texture(iChannel0, uv*vec2(1, 1));
#elif FILTER==1
    // horizontal
    texture( iChannel0, uv*vec2(BOT, BOT))
    +texture(iChannel0, uv*vec2(CEN, BOT)) *2.0
    +texture(iChannel0, uv*vec2(TOP, BOT))
    -texture(iChannel0, uv*vec2(BOT, TOP))
    -texture(iChannel0, uv*vec2(CEN, TOP)) *2.0
    -texture(iChannel0, uv*vec2(TOP, TOP));
#elif FILTER==2
    // vertical edges
    texture( iChannel0, uv*vec2(BOT, BOT))
    +texture(iChannel0, uv*vec2(BOT, CEN)) *2.0
    +texture(iChannel0, uv*vec2(BOT, TOP))
    -texture(iChannel0, uv*vec2(TOP, BOT))
    -texture(iChannel0, uv*vec2(TOP, CEN)) *2.0
    -texture(iChannel0, uv*vec2(TOP, TOP));
#elif FILTER==3
    // all
    texture( iChannel0, uv) *4.
    -texture(iChannel0, uv*vec2(CEN, BOT))
    -texture(iChannel0, uv*vec2(BOT, CEN))
    -texture(iChannel0, uv*vec2(TOP, CEN))
    -texture(iChannel0, uv*vec2(CEN, TOP));
#elif FILTER==4
    // sharpen
    texture( iChannel0, uv) *2.
    -texture(iChannel0, uv*vec2(BOT, BOT))/8.
    -texture(iChannel0, uv*vec2(CEN, BOT))/8.
    -texture(iChannel0, uv*vec2(TOP, BOT))/8.
    -texture(iChannel0, uv*vec2(BOT, CEN))/8.
    -texture(iChannel0, uv*vec2(TOP, CEN))/8.
    -texture(iChannel0, uv*vec2(BOT, TOP))/8.
    -texture(iChannel0, uv*vec2(CEN, TOP))/8.
    -texture(iChannel0, uv*vec2(TOP, TOP))/8.;
#elif FILTER==5
    // blur
    texture( iChannel0, uv*vec2(BOT, BOT))/8.
    +texture(iChannel0, uv*vec2(BOT, BOT))/8.
    +texture(iChannel0, uv*vec2(TOP, BOT))/8.
    +texture(iChannel0, uv*vec2(BOT, CEN))/8.
    +texture(iChannel0, uv*vec2(TOP, CEN))/8.
    +texture(iChannel0, uv*vec2(BOT, TOP))/8.
    +texture(iChannel0, uv*vec2(CEN, TOP))/8.
    +texture(iChannel0, uv*vec2(TOP, TOP))/8.;
#else
    // sharpen more
    texture( iChannel0, uv) *5.
    -texture(iChannel0, uv*vec2(CEN, BOT))
    -texture(iChannel0, uv*vec2(BOT, CEN))
    -texture(iChannel0, uv*vec2(TOP, CEN))
    -texture(iChannel0, uv*vec2(CEN, TOP));
#endif
}
