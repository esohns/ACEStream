#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

#define NUM_POINTS 2048
#define SEED 3

void
main ()
{
  int random = SEED;
    
  int a = 1103515245;
  int c = 12345;
  int m = 2147483648;
    
  vec2 o;
    
  float minDist = 10000000.0;
  float dist = minDist;
  for (int i = 0; i < NUM_POINTS; i++)
  {
    random = a * random + c;

    o.x = (float(random) / float(m)) * iResolution.x;

    random = a * random + c;

    o.y = (float(random) / float(m)) * iResolution.y;

    dist = distance(gl_FragCoord.xy, o);
    if (dist < minDist)
    {
      minDist = dist;
      vec2 uv = o / iResolution.xy;
      gl_FragColor = (texture(iChannel0, uv)) * (1.0 - minDist / 200.0);
    }
  }
}
