#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;

void
main ()
{
  float time = iTime;
  vec2 uv = gl_FragCoord.xy / iResolution.xy;

  vec2 pixelSize = vec2(1, 1) / iResolution.xy;

  vec3 col = texture(iChannel0, uv).rgb;
  float mirrorPos = 0.3;
  if (uv.y < mirrorPos)
  {
    float distanceFromMirror = mirrorPos - uv.y;
    float sine = sin((log(distanceFromMirror) * 20.0) + (iTime * 2.0));
    float dy = 30.0 * sine;
    float dx = 0.0;
    dy *= distanceFromMirror;
    vec2 pixelOff = pixelSize * vec2(dx, dy);
    vec2 tex_uv = uv + pixelOff;
    tex_uv.y = (0.6) - tex_uv.y;
    col = texture(iChannel0, tex_uv).rgb;

    float shine = (sine + dx * 0.05) * 0.05;
    col += vec3(shine, shine, shine);
  }

  gl_FragColor = vec4(col, 1.);
}
