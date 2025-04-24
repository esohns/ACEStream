#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;

#define r 0.3
#define rep 3.0

void
main ()
{
  float R = r * iResolution.x; // radius of rolling cylinder

  float v = 1.5 * iResolution.x / rep;

  float time = fract(iTime / rep);

  vec2 s = gl_FragCoord.xy; // pixel coordinates

  vec2 u = normalize(vec2(5.0, 1.0)); // direction of movement

  vec2 o = vec2(time *rep* v, 0.0); // origin of cylinder

  float d = dot(s - o, u); // distance to generator of cylinder

  vec2 h = s - u * d; // projection on generator

  bool onCylinder = abs(d) < R;

  float angle = onCylinder ? asin(d / R) : 0.0;

  bool neg = d < 0.0;

  float a0 = 3.141592653 + angle;

  float a = onCylinder ? (neg ? -angle : (3.141592653 + angle)) : 0.0; // angle

  float l = R * a; // length of arc

  vec2 p = h - u * l; // unwrapped point from cylinder to plane

  bool outside = any(lessThan(p, vec2(0.0))) || any(greaterThan(p, iResolution.xy));

  bool previous = (!onCylinder ||outside) && neg;

  bool page = !onCylinder || outside;

  vec4 color;
  if (page)
    color = texture(iChannel0, gl_FragCoord.xy / iResolution.xy);
  else
    color = texture(iChannel0, p / iResolution.xy);
  color *= (previous ? mix(0.1, 1.0, time): 1.0);

  l = R * a0; // length of arc

  p = h - u * l; // unwrapped point from cylinder to plane

  outside = any(lessThan(p, vec2(0.0))) || any(greaterThan(p, iResolution.xy));

  color = outside || !onCylinder ? color : texture(iChannel0, p / iResolution.xy);

  gl_FragColor = color;
}
