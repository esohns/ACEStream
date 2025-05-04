#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;

float
hash (vec2 p)
{
  return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x))));
}

float
compare (vec3 a, vec3 b)
{
  a = max(vec3(0.0), a - min(a.r, min(a.g, a.b)) * 0.25);
  b = max(vec3(0.0), b - min(b.r, min(b.g, b.b)) * 0.25);
  a *= a * a;
  b *= b * b;
  vec3 diff = (a - b);
  return dot(diff, diff);
}

void
main ()
{
  const float pixelSize = 4.0;
  vec2 c = floor(gl_FragCoord.xy / pixelSize);
  vec2 coord = c * pixelSize;
  vec3 src = texture(iChannel0, coord / iResolution.xy).rgb;

  vec3 dst0 = vec3(0), dst1 = vec3(0);
  float best0 = 1e3, best1 = 1e3;
#define TRY(R, G, B) { const vec3 tst = vec3(R, G, B); float err = compare(src, tst); if (err < best0) { best1 = best0; dst1 = dst0; best0 = err; dst0 = tst; } }
  TRY(0.078431, 0.047059, 0.109804);
  TRY(0.266667, 0.141176, 0.203922);
  TRY(0.188235, 0.203922, 0.427451);
  TRY(0.305882, 0.290196, 0.305882);
  TRY(0.521569, 0.298039, 0.188235);
  TRY(0.203922, 0.396078, 0.141176);
  TRY(0.815686, 0.274510, 0.282353);
  TRY(0.458824, 0.443137, 0.380392);
  TRY(0.349020, 0.490196, 0.807843);
  TRY(0.823529, 0.490196, 0.172549);
  TRY(0.521569, 0.584314, 0.631373);
  TRY(0.427451, 0.666667, 0.172549);
  TRY(0.823529, 0.666667, 0.600000);
  TRY(0.427451, 0.760784, 0.792157);
  TRY(0.854902, 0.831373, 0.368627);
  TRY(0.870588, 0.933333, 0.839216);
#undef TRY

  best0 = sqrt(best0);
  best1 = sqrt(best1);
  gl_FragColor = vec4(mod(c.x + c.y, 2.0) > (hash(c * 2.0 + fract(sin(vec2(floor(iTime), floor(iTime * 1.7))))) * 0.75) + (best1 / (best0 + best1)) ? dst1 : dst0, 1.0);
}
