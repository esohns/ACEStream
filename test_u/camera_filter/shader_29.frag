#version 130

uniform vec2 iResolution;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

void
main ()
{
  float ChromaticAberration = iMouse.y / 10.0 + 8.0;
  vec2 uv = gl_FragCoord.xy / iResolution.xy;

  vec2 texel = 1.0 / iResolution.xy;

  vec2 coords = (uv - 0.5) * 2.0;
  float coordDot = dot (coords, coords);

  vec2 precompute = ChromaticAberration * coordDot * coords;
  vec2 uvR = uv - texel.xy * precompute;
  vec2 uvB = uv + texel.xy * precompute;

  vec4 color;
  color.r = texture(iChannel0, uvR).r;
  color.g = texture(iChannel0, uv).g;
  color.b = texture(iChannel0, uvB).b;
  color.a = 1.0;

  gl_FragColor = color;
}
