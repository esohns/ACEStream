#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

float gamma = 0.55;

float
Cubic (float value)
{
  if (value < 0.5)
    return value * value * value * value * value * 16.0; 

  value -= 1.0;

  return value * value * value * value * value * 16.0 + 1.0;
}

float
Sigmoid (float x)
{
  return 1.0 / (1.0 + (exp(-(x - 0.5) * 14.0))); 
}

void
main ()
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy;

  vec4 C = texture(iChannel0, uv);

  C = vec4(Cubic(C.r), Cubic(C.g),Cubic(C.b), 1.0); 
  //C = vec4(Sigmoid(C.r), Sigmoid(C.g),Sigmoid(C.b), 1.0); 

  C = pow(C, vec4(gamma)); 

  gl_FragColor = C;
}
