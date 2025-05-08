#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform int iFrame;
uniform vec4 iMouse;
uniform sampler2D iChannel0; // buffer A
uniform sampler2D iChannel1; // random noise
uniform sampler2D iChannel2; // video

#define Xnum 4
#define Ynum 4

#define N(v) (v.yx*vec2(1,-1))

#define RotNum 5
//#define SUPPORT_EVEN_ROTNUM

#define Res  vec2(textureSize(iChannel0,0))
#define Res1 vec2(textureSize(iChannel1,0))

const float ang = 2.0 * 3.1415926535 / float(RotNum);
mat2 m = mat2(cos(ang), sin(ang), -sin(ang), cos(ang));
mat2 mh = mat2(cos(ang * 0.5), sin(ang * 0.5), -sin(ang * 0.5), cos(ang * 0.5));

vec4
getRand (vec2 coord)
{
  return texture(iChannel1, coord.xy / Res1.xy);
}

vec4
randS (vec2 uv)
{
  return texture(iChannel1, uv * Res.xy / Res1.xy) - vec4(0.5);
}

vec4
getCol (vec2 coord)
{
  return texture(iChannel0, coord / Res.xy);
}

float
getVal (vec2 coord)
{
  vec4 c = getCol(coord);
  return dot(c.xyz, vec3(.3333));
  return dot(c.xyz, c.xyz);
  return length(getCol(coord));
}

vec2
getGrad (vec2 coord, float eps)
{
  vec2 d = vec2(eps, 0);
  return vec2(getVal(coord + d.xy) - getVal(coord - d.xy),
              getVal(coord + d.yx) - getVal(coord - d.yx)) / eps;
}

float
getRot (vec2 pos, vec2 b)
{
  vec2 p = b;
  float rot = 0.0;
  for (int i = 0; i < RotNum; i++)
  {
    rot += dot(texture(iChannel0, fract((pos + p) / Res.xy)).xy - vec2(0.5), p.yx * vec2(1, -1));
    p = m * p;
  }
  return rot / float(RotNum) / dot(b, b);
}

void
effectFlow (inout vec4 fragColor, vec2 pos)
{
  float rnd = randS(vec2(float(iFrame) / Res.x, 0.5 / Res1.y)).x * 1.;
    
  vec2 b = vec2(cos(ang * rnd), sin(ang * rnd));
  vec2 v = vec2(0);
  float bbMax = 0.7 * Res.y;
  bbMax *= bbMax;
  for (int l = 0; l < 3; l++)
  {
    if (dot(b, b) > bbMax)
      break;
    vec2 p = b;
    for (int i = 0; i < RotNum; i++)
    {
#ifdef SUPPORT_EVEN_ROTNUM
      v+=p.yx*getRot(pos+p,-mh*b);
#else
      // this is faster but works only for odd RotNum
      v += p.yx * getRot(pos + p, b);
#endif
      p = m * p;
    }
    b *= 2.0;
  }
    
  fragColor = getCol(pos + v * vec2(-1, 1) * 4.);
}

void
effectSmear (inout vec4 col, vec2 coord)
{
  vec2 g = getGrad(coord, .5);
  col = getCol(coord + g.yx * vec2(1, -1) * .7);
}

void
effectDiff (inout vec4 col, vec2 coord)
{
  vec2 g = getGrad(coord, .5);
  col = getCol(coord + g.xy * 1.5 * iResolution.x / 600.);
}

void
effect (inout vec4 col, vec2 coord)
{
  vec4 col1, col2, col3;
  effectFlow(col1, coord);
  effectSmear(col2, coord);
  effectDiff(col3, coord);
  float effType = smoothstep(.0, .2, -sin(iTime * .3 - .3));
  if (iMouse.y > 1.)
    effType = iMouse.y / iResolution.y;
  col = mix(col1, col3, effType);
  col = mix(col, col2, .3);
}

void
main ()
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy * vec2(Xnum, Ynum);

  vec2 uv2 = vec2(mod(uv.x - 1. + float(Xnum), float(Xnum)),
                      uv.y - (((uv.x - 1.) < 0.) ? 1. : 0.)) / vec2(Xnum, Ynum);

  vec4 c1, c2;
  effect(c1, uv2 * iResolution.xy);
  effect(c2, gl_FragCoord.xy);
    
  if (gl_FragCoord.x < iResolution.x / float(Xnum) && gl_FragCoord.y < iResolution.y / float(Ynum))
  {
    vec4 col = texture(iChannel2, uv);
    float bgfact = dot(col.xyz, vec3(-1, 2, -1));
    vec4 bg = vec4(1, .95, .75, 1);
    bg = texture(iChannel1, gl_FragCoord.xy / iResolution.xy * .2).xyzw * .3 + bg * .0;
    c1 = mix(col, bg, bgfact);
    c1.w = bgfact;
  }

  gl_FragColor = mix(c1, c2, smoothstep(.4, .5, c1.w));
}
