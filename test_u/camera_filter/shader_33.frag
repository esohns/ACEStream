#version 130

uniform vec2 iResolution;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

const float PI = 3.1415926535897932384626433832795;
const float PI180 = float(PI / 180.0);

float
sind (float a)
{
  return sin (a * PI180);
}

float
cosd (float a)
{
  return cos (a * PI180);
}

float
added (vec2 sh, float sa, float ca, vec2 c, float d)
{
  return 0.5 + 0.25 * cos((sh.x * sa + sh.y * ca + c.x) * d) + 0.25 * cos((sh.x * ca - sh.y * sa + c.y) * d);
}

void
main ()
{
  float threshold = clamp(float(iMouse.x / iResolution.x) + 0.6, 0.0, 1.0);

  float ratio = iResolution.y / iResolution.x;
  float coordX = gl_FragCoord.x / iResolution.x;
  float coordY = gl_FragCoord.y / iResolution.x;
  vec2 dstCoord = vec2(coordX, coordY);
  vec2 srcCoord = vec2(coordX, coordY / ratio);
  vec2 rotationCenter = vec2(0.5, 0.5);
  vec2 shift = dstCoord - rotationCenter;

  float dotSize = 3.0;
  float angle = 45.0;

  float rasterPattern = added(shift, sind(angle), cosd(angle), rotationCenter, PI / dotSize * 680.0);
  vec4 srcPixel = texture(iChannel0, srcCoord);

  float avg = 0.2125 * srcPixel.r + 0.7154 * srcPixel.g + 0.0721 * srcPixel.b;
  float gray = (rasterPattern * threshold + avg - threshold) / (1.0 - threshold);
// gray = rasterPattern;

  gl_FragColor = vec4(gray, gray, gray, 1.0);
}
