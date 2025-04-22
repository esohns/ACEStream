#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

#define LUM vec3(.2126, .7152, .0722)

//#define CHROMA_KEY
#define CHROMA_BIAS (0.13)
#define SCALE       (1.00)
#define BGCOLOR     vec3(0.03, 0.1, 0.2)

void
main ()
{
  vec4 c;
  vec2 p = gl_FragCoord.xy;

  float s = (SCALE * (iResolution.x / 6e1));
  c = texture(iChannel0, floor((p + .5) / s) * s / iResolution.xy);
#ifdef CHROMA_KEY
  float lum = dot(LUM, c.rgb);
  if (lum > max(c.r, c.b) + CHROMA_BIAS)
    c = vec4(BGCOLOR,1);
#endif /* CHROMA_KEY */

  gl_FragColor = c;
}
