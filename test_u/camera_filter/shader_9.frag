#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

// Params = (wave frequency in Hz, number of waves per unit distance)
//
vec2 params = vec2(2.5, 10.0);

float
wave (vec2 pos, float t, float freq, float numWaves, vec2 center)
{
  float d = length(pos - center);
  d = log(1.0 + exp(d));
  return 1.0/(1.0+20.0*d*d) * sin(2.0*3.1415*(-numWaves*d + t*freq));
}

float
height (vec2 pos, float t)
{
  float w;
  w =  wave(pos, t, params.x, params.y, vec2(0.5, -0.5));
  w += wave(pos, t, params.x, params.y, -vec2(0.5, -0.5));
  return w;
}

vec2
normal (vec2 pos, float t)
{
  return 	vec2(height(pos - vec2(0.01, 0), t) - height(pos, t), 
               height(pos - vec2(0, 0.01), t) - height(pos, t));
}

void
main ()
{
//  if (iMouse.z > 0.0)
    params = 2.0*params*iMouse.xy/iResolution.xy;

  vec2 uv = gl_FragCoord.xy / iResolution.xy;
  vec2 uvn = 2.0*uv - vec2(1.0);	
  uv += normal(uvn, iTime);
  gl_FragColor = texture(iChannel0, vec2(1.0-uv.x, uv.y));
}
