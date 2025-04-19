#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

vec2 tile_num = vec2 (40.0, 20.0);

void
main ()
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy;
  vec2 uv2 = floor(uv*tile_num)/tile_num;
  uv -= uv2;
  uv *= tile_num;
  gl_FragColor = texture(iChannel0, uv2 + vec2(step(1.0-uv.y,uv.x)/(2.0*tile_num.x),
                              //0,
                              step(uv.x,uv.y)/(2.0*tile_num.y)
                              //0
                                                   ) );
}
