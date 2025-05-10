#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

void
main ()
{
  vec2 res = iResolution.xy;
  vec2 tc = gl_FragCoord.xy / res;
  vec2 uv = tc;

  uv *= 0.998;

  vec4 sum = texture(iChannel1, uv);
  vec4 src = texture(iChannel0, tc);

  sum.rgb = mix(sum.rbg, src.rgb, 0.01);
  gl_FragColor = sum;
}
