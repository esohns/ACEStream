#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

float th = 8.0;
float a2 = 1.2;
float spill = 1.0;

float
getAlpha (vec4 c)
{
  return 1.0 - th * (c.g - a2 * (max(c.r, c.b)));
}

vec4
despill (vec4 c)
{
  float sub = max(c.g - mix(c.b, c.r, 0.45), 0.0);
  c.g -= sub;

  c.a -= smoothstep(0.25, 0.5, sub * c.a);

  float luma = dot(c.rgb, vec3(0.350, 0.587, 0.164));
  c.r += sub * c.r * 2.0 * .350 / luma;
  c.g += sub * c.g * 2.0 * .587 / luma;
  c.b += sub * c.b * 2.0 * .164 / luma;

  return c;
}

void
main ()
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy;
  vec4 fg = texture(iChannel0, uv);

  vec4 ofg = fg;

  fg.a = clamp(getAlpha(fg), 0.0, 1.0);
  fg = despill(fg);
  gl_FragColor = fg.aaaa;
}
