#version 130

uniform vec2 iResolution;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

vec2
fisheye_lookup (float fov, vec2 position)
{
  vec2 d = position - 0.5;
  
  float yaw = sqrt(d.x*d.x+d.y*d.y) * fov;

  float roll = -atan(d.y, d.x);
  float sx = sin(yaw) * cos(roll);
  float sy = sin(yaw) * sin(roll);
  float sz = cos(yaw);

  return vec2(sx, sy);
}

void
main ()
{
  vec2 p = gl_FragCoord.xy / iResolution.xy;
  vec2 m = iMouse.xy / iResolution.xy;
  float lensSize = 2.0;
  float scale = 1.0;

  vec2 d = p - m;
  float r = scale * sqrt(dot(d, d));
  float rThresh = 0.3;

  vec2 pos = d;
  float apertureHalf = 0.5 * 90.0 * (3.14159 / 180.0);
  float maxFactor = sin(apertureHalf);

  vec2 uv;

  float avgD = (iResolution.x + iResolution.y)/2.0;
  vec2 fMinMouse = gl_FragCoord.xy/avgD - iMouse.xy/iResolution.xy;
  float r2 = scale * sqrt(dot(fMinMouse,fMinMouse));

  if (r2 >= lensSize)
    gl_FragColor = vec4(0,0,0, 1.0);
  else
  {
    vec2 p2 = gl_FragCoord.xy / iResolution.x;
    float prop = iResolution.x / iResolution.y;
    vec2 m2 = vec2(0.5, 0.5 / prop);
    vec2 d2 = p2 - m2;
    float r2 = sqrt(dot(d2, d2));

    float power = ( 2.0 * 3.141592 / (2.0 * sqrt(dot(m2, m2))) ) * (m.x - 0.5);

    float bind;
    if (power > 0.0)
      bind = sqrt(dot(m2, m2));
    else if (prop < 1.0)
      bind = m2.x;
    else
      bind = m2.y;

    if (power > 0.0)
      uv = m2 + normalize(d2) * tan(r2 * power) * bind / tan( bind * power);
    else if (power < 0.0)
      uv = m2 + normalize(d2) * atan(r2 * -power * 10.0) * bind / atan(-power * bind * 10.0);
    else
      uv = p;

    vec3 col = texture(iChannel0, vec2(uv.x, uv.y * prop)).xyz;
    gl_FragColor = vec4(col, 1.0);
    //gl_FragColor = vec4(texture(iChannel0, uv).xyz, 1.0);
  }
}
