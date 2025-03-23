#version 130

uniform vec2 iResolution;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

#define PI 3.14159

void
main ()
{
  float effectRadius = .5;
  float effectAngle = 2. * PI;

  vec2 center = iMouse.xy / iResolution.xy;
  center = center == vec2(0., 0.) ? vec2(.5, .5) : center;

  vec2 uv = gl_FragCoord.xy / iResolution.xy - center;

  float len = length(uv * vec2(iResolution.x / iResolution.y, 1.));
  float angle = atan(uv.y, uv.x) + effectAngle * smoothstep(effectRadius, 0., len);
  float radius = length(uv);

  gl_FragColor = texture(iChannel0, vec2(radius * cos(angle), radius * sin(angle)) + center);
}
