#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

const int _Steps = 64;
const vec3 lightDir = vec3(0.577, 0.577, 0.577);

vec3
rotateX (vec3 p, float a)
{
  float sa = sin(a);
  float ca = cos(a);
  vec3 r;
  r.x = p.x;
  r.y = ca * p.y - sa * p.z;
  r.z = sa * p.y + ca * p.z;
  return r;
}

vec3
rotateY (vec3 p, float a)
{
  float sa = sin(a);
  float ca = cos(a);
  vec3 r;
  r.x = ca * p.x + sa * p.z;
  r.y = p.y;
  r.z = -sa * p.x + ca * p.z;
  return r;
}

bool
intersectBox (vec3 ro, vec3 rd, vec3 boxmin, vec3 boxmax, out float tnear, out float tfar)
{
  vec3 invR = 1.0 / rd;
  vec3 tbot = invR * (boxmin - ro);
  vec3 ttop = invR * (boxmax - ro);

  vec3 tmin = min(ttop, tbot);
  vec3 tmax = max(ttop, tbot);

  vec2 t0 = max(tmin.xx, tmin.yz);
  tnear = max(t0.x, t0.y);
  t0 = min(tmax.xx, tmax.yz);
  tfar = min(t0.x, t0.y);

  bool hit;
  if (tnear > tfar) 
    hit = false;
  else
    hit = true;
  return hit;
}

float
luminance (sampler2D tex, vec2 uv)
{
  vec3 c = textureLod(tex, uv, 0.0).xyz;
  return dot(c, vec3(0.33, 0.33, 0.33));
}

vec2
gradient (sampler2D tex, vec2 uv, vec2 texelSize)
{
  float h = luminance(tex, uv);
  float hx = luminance(tex, uv + texelSize * vec2(1.0, 0.0));
  float hy = luminance(tex, uv + texelSize * vec2(0.0, 1.0));
  return vec2(hx - h, hy - h);
}

vec2
worldToTex (vec3 p)
{
  vec2 uv = p.xz * 0.5 + 0.5;
  uv.y = 1.0 - uv.y;
  return uv;
}

float
heightField (vec3 p)
{
  return luminance(iChannel0, worldToTex(p)) * 0.5;
}

bool
traceHeightField (vec3 ro, vec3 rayStep, out vec3 hitPos)
{
  vec3 p = ro;
  bool hit = false;
  float pH = 0.0;
  vec3 pP = p;
  for (int i = 0; i < _Steps; i++)
  {
    float h = heightField(p);
    if ((p.y < h) && !hit)
    {
      hit = true;
      //hitPos = p;
      // interpolate based on height
      hitPos = mix(pP, p, (pH - pP.y) / ((p.y - pP.y) - (h - pH)));
    }
    pH = h;
    pP = p;
    p += rayStep;
  }
  return hit;
}

vec3
background (vec3 rd)
{
  return mix(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.5, 1.0), abs(rd.y));
}

void
main ()
{
  vec2 pixel = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;

  float asp = iResolution.x / iResolution.y;
  vec3 rd = normalize(vec3(asp * pixel.x, pixel.y, -2.0));
  vec3 ro = vec3(0.0, 0.0, 2.0);

  vec2 mouse = iMouse.xy / iResolution.xy;

  float ax = -0.7;
  if (iMouse.x > 0.0)
    ax = -(1.0 - mouse.y) * 2.0 - 1.0;
  rd = rotateX(rd, ax);
  ro = rotateX(ro, ax);
    
  float ay = sin(iTime * 0.2);
  rd = rotateY(rd, ay);
  ro = rotateY(ro, ay);
  
  bool hit;
  const vec3 boxMin = vec3(-1.0, -0.01, -1.0);
  const vec3 boxMax = vec3(1.0, 0.5, 1.0);
  float tnear, tfar;
  hit = intersectBox(ro, rd, boxMin, boxMax, tnear, tfar);

  tnear -= 0.0001;
  vec3 pnear = ro + rd * tnear;
  vec3 pfar = ro + rd * tfar;
  
  float stepSize = length(pfar - pnear) / float(_Steps);
  
  vec3 rgb = background(rd);
  if (hit)
  {
    ro = pnear;
    vec3 hitPos;
    hit = traceHeightField(ro, rd * stepSize, hitPos);
    if (hit)
    {
      vec2 uv = worldToTex(hitPos);
      rgb = texture(iChannel0, uv).xyz;

#if 0
      hitPos += vec3(0.0, 0.01, 0.0);
      bool shadow = traceHeightField(hitPos, lightDir*0.01, hitPos);
      if (shadow)
        rgb *= 0.75;
#endif
    }
  }

  gl_FragColor = vec4(rgb, 1.0);
}
