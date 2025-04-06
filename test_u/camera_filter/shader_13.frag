#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

#define SHARPEN_FACTOR 16.0

vec4
sharpenMask (sampler2D tex, vec2 fragCoord)
{
  vec4 up = texture (tex, (fragCoord + vec2 (0, 1))/iResolution.xy);
  vec4 left = texture (tex, (fragCoord + vec2 (-1, 0))/iResolution.xy);
  vec4 center = texture (tex, fragCoord/iResolution.xy);
  vec4 right = texture (tex, (fragCoord + vec2 (1, 0))/iResolution.xy);
  vec4 down = texture (tex, (fragCoord + vec2 (0, -1))/iResolution.xy);

  return (1.0 + 4.0*SHARPEN_FACTOR)*center -SHARPEN_FACTOR*(up + left + right + down);
}

void
main ()
{
  gl_FragColor = sharpenMask (iChannel0, gl_FragCoord.xy);
}
