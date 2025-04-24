#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

const float Soft = 0.001;
const float Threshold = 0.3;

void
main ()
{
  float f = Soft/2.0;
  float a = Threshold - f;
  float b = Threshold + f;

  vec4 tx = texture(iChannel0, gl_FragCoord.xy/iResolution.xy);
  float l = (tx.x + tx.y + tx.z) / 3.0;
  float v = smoothstep(a, b, l);

  gl_FragColor = vec4 (v, v, v, 1.0);
}
