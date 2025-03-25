#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

float curlExtent = 0.;
    
const float minAmount = -0.16;
const float maxAmount = 1.3;
const float PI = 3.141592653589793;
const float scale = 512.0;
const float sharpness = 3.0;
vec4 bgColor;

float amount;
float cylinderCenter;
float cylinderAngle;
const float cylinderRadius = 1.0 / PI / 2.0;

vec3
hitPoint (float hitAngle, float yc, vec3 point, mat3 rrotation)
{
  float hitPoint = hitAngle / (2.0 * PI);
  point.y = hitPoint;
  return rrotation * point;
}

vec4
antiAlias (vec4 color1, vec4 color2, float distance)
{
  distance *= scale;
  if (distance < 0.0) return color2;
  if (distance > 2.0) return color1;
  float dd = pow(1.0 - distance / 2.0, sharpness);
  return ((color2 - color1) * dd) + color1;
}

float
distanceToEdge (vec3 point)
{
  float dx = abs(point.x > 0.5 ? 1.0 - point.x : point.x);
  float dy = abs(point.y > 0.5 ? 1.0 - point.y : point.y);
  if (point.x < 0.0) dx = -point.x;
  if (point.x > 1.0) dx = point.x - 1.0;
  if (point.y < 0.0) dy = -point.y;
  if (point.y > 1.0) dy = point.y - 1.0;
  if ((point.x < 0.0 || point.x > 1.0) && (point.y < 0.0 || point.y > 1.0)) return sqrt(dx * dx + dy * dy);
  return min(dx, dy);
}

vec4
seeThrough (float yc, vec2 p, mat3 rotation, mat3 rrotation)
{
  float hitAngle = PI - (acos(yc / cylinderRadius) - cylinderAngle);
  vec3 point = hitPoint(hitAngle, yc, rotation * vec3(p, 1.0), rrotation);
  if (yc <= 0.0 && (point.x < 0.0 || point.y < 0.0 || point.x > 1.0 || point.y > 1.0))
    return bgColor;
  if (yc > 0.0)
    return texture(iChannel0, p);
  vec4 color = texture(iChannel0, point.xy);
  vec4 tcolor = vec4(0.0);
  return antiAlias(color, tcolor, distanceToEdge(point));
}

vec4
seeThroughWithShadow (float yc, vec2 p, vec3 point, mat3 rotation, mat3 rrotation)
{
  float shadow = distanceToEdge(point) * 30.0;
  shadow = (1.0 - shadow) / 3.0;
  if (shadow < 0.0)
    shadow = 0.0;
  else
    shadow *= amount;
  vec4 shadowColor = seeThrough(yc, p, rotation, rrotation);
  shadowColor.r -= shadow;
  shadowColor.g -= shadow;
  shadowColor.b -= shadow;
  return shadowColor;
}

vec4
backside (float yc, vec3 point)
{
  vec4 color = texture(iChannel0, point.xy);
  float gray = (color.r + color.b + color.g) / 15.0;
  gray += (8.0 / 10.0) * (pow(1.0 - abs(yc / cylinderRadius), 2.0 / 10.0) / 2.0 + (5.0 / 10.0));
  color.rgb = vec3(gray);
  return color;
}

void
main ()
{
  vec2 uv = gl_FragCoord.xy/iResolution.xy;

  bgColor = texture(iChannel0, uv).rgga;

  curlExtent = sin(iTime*0.3)*0.5+0.5;

  if (iMouse.z>0.)
    curlExtent = iMouse.y/iResolution.y;

  amount = curlExtent * (maxAmount - minAmount) + minAmount;
  cylinderCenter = amount;
  cylinderAngle = 2.0 * PI * amount;

  const float angle = 30.0 * PI / 180.0;
  float c = cos(-angle);
  float s = sin(-angle);
  mat3 rotation = mat3(c, s, 0, -s, c, 0, 0.12, 0.258, 1);
  c = cos(angle);
  s = sin(angle);
  mat3 rrotation = mat3(c, s, 0, -s, c, 0, 0.15, -0.5, 1);
  vec3 point = rotation * vec3(uv, 1.0);
  float yc = point.y - cylinderCenter;
  vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
  if (yc < -cylinderRadius)
  {
    color = bgColor;
  } 
  else if (yc > cylinderRadius)
  {
    color = texture(iChannel0, uv);
  } 
  else 
  {
    float hitAngle = (acos(yc / cylinderRadius) + cylinderAngle) - PI;
    float hitAngleMod = mod(hitAngle, 2.0 * PI);
    if ((hitAngleMod > PI && amount < 0.5) || (hitAngleMod > PI/2.0 && amount < 0.0)) 
    {
      color = seeThrough(yc, uv, rotation, rrotation);
    } 
    else 
    {
      point = hitPoint(hitAngle, yc, point, rrotation);
      if (point.x < 0.0 || point.y < 0.0 || point.x > 1.0 || point.y > 1.0) 
      {
        color = seeThroughWithShadow(yc, uv, point, rotation, rrotation);
      } 
      else 
      {
        color = backside(yc, point);
        vec4 otherColor;
        if (yc < 0.0) 
        {
          float shado = 1.0 - (sqrt(pow(point.x - 0.5, 2.0) + pow(point.y - 0.5, 2.0)) / 0.71);
          shado *= pow(-yc / cylinderRadius, 3.0);
          shado *= 0.5;
          otherColor = vec4(0.0, 0.0, 0.0, shado);
        } 
        else 
        {
          otherColor = texture(iChannel0, uv);
        }
        color = antiAlias(color, otherColor, cylinderRadius - abs(yc));
      }
    }
  }
  gl_FragColor = color;
}
