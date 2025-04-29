#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

#define L 8.  // interline distance
#define A 4.  // amplification factor
#define P 6.  // thickness

void
main ()
{
  vec4 o = vec4(0.);
  vec2 uv = gl_FragCoord.xy;
  
  uv /= L;
  vec2 p = floor(uv + .5);

#define T(x,y) texture(iChannel0,L*vec2(x,y)/iResolution.xy).g
#define M(c,T) o += pow(.5+.5*cos( 6.28*(uv-p).c + A*(2.*T-1.) ),P)

  M(y, T( uv.x, p.y ));
  M(x, T( p.x, uv.y ));

  gl_FragColor = o;
}
