#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

void
main ()
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy;
  gl_FragColor = texture(iChannel0, uv);
}
