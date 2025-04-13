#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

#define PI2 6.28318530718
#define CS(x) cos(x-vec2(0,PI2/4.))
#define N(v) (v.yx*vec2(-1,1))
#define ROT2(x) mat2(CS(x),N(CS(x)))
#define MAP2PI(ang) ((ang<PI2/2.)?PI2-ang:ang)

//#define Res (iResolution.xy)
#define Res0 vec2(textureSize(iChannel0,0))
#define Res1 vec2(textureSize(iChannel1,0))
#define Res2 vec2(textureSize(iChannel2,0))

uniform float SrcContrast;
uniform float SrcBright;

vec4
getRand (vec2 pos)
{
  return textureLod(iChannel1,pos/Res1,0.);
}

vec4
getRand (int idx)
{
  ivec2 rres=textureSize(iChannel1,0);
  idx=idx%(rres.x*rres.y);
  return texelFetch(iChannel1,ivec2(idx%rres.x,idx/rres.x),0);
}

vec4
getCol (vec2 pos, float lod)
{
  vec2 tres = Res0;
  vec2 tpos = (pos-.5*Res2)*min(tres.y/Res2.y,tres.x/Res2.x);
  vec2 uv = (tpos+tres*.5)/tres;
  vec4 bg=textureLod(iChannel3,uv+.0*sin(iTime*vec2(1,.7)+vec2(0,1.6)),lod).xxxx*.5+.2;
  vec4 fg=textureLod(iChannel0,uv,lod);
  return mix(fg,bg,dot(fg.xyz,vec3(-.7,1.4,-.7)));
}

float
getVal (vec2 pos)
{
  return dot(getCol(pos,0.).xyz,vec3(1)/3.);
}

vec3
getValCol (vec2 pos, float lod)
{
  return getCol(pos,lod).xyz;
}

float
compsignedmax (vec3 c)
{
  vec3 s=sign(c);
  vec3 a=abs(c);
  if (a.x>a.y && a.x>a.z) return c.x;
  if (a.y>a.x && a.y>a.z) return c.y;
  return c.z;
}

vec2
getGradMax (vec2 pos, float eps, float lod)
{
  eps*=Res2.x/Res0.x*pow(2.,lod);
  vec2 d=vec2(eps,0);
  return vec2(compsignedmax(getValCol(pos+d.xy,lod)-getValCol(pos-d.xy,lod)),
              compsignedmax(getValCol(pos+d.yx,lod)-getValCol(pos-d.yx,lod))
             )/eps/2.;
}

vec2
getGradBr (vec2 pos, float eps, float lod)
{
  eps*=Res2.x/Res0.x*pow(2.,lod);
  vec2 d=vec2(eps,0);
  return vec2(dot(getValCol(pos+d.xy,lod)-getValCol(pos-d.xy,lod),vec3(.333)),
              dot(getValCol(pos+d.yx,lod)-getValCol(pos-d.yx,lod),vec3(.333))
             )/eps/2.;
}

vec2
getGrad (vec2 pos, float eps, float lod)
{
  return getGradMax(pos,eps,lod);
}

float
getDetail (vec2 pos, float lod)
{
  float d = (length(getGrad(pos,1.,lod-.5))-length(getGrad(pos,1.,lod+.5)))*Res2.x/600.*pow(2.,lod);
  return d;
  //return smoothstep(.04,.05,d)*.25;
}

float
quant (float x, float num)
{
  return floor(x*(num-.0001))/(num-1.);
}

float
getDetailAll (vec2 pos)
{
  float d = 0.
    +getDetail(pos,0.)
    +getDetail(pos,1.)
    +getDetail(pos,2.)
    +getDetail(pos,3.)
    +getDetail(pos,4.)
    +getDetail(pos,5.)
  //+getDetail(pos,6.)
  ;
  return d;
}

float
getCurl (vec2 pos, float eps)
{
  return 1.;
}

void
main ()
{
  vec2 pos = gl_FragCoord.xy;
  vec2 g = getGrad(pos,1.4,0.);
  float curl = getCurl(pos,1.4);
  float detail = getDetailAll(pos);
  gl_FragColor = vec4(g,detail,curl);
}
