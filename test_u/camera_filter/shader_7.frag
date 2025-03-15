#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0; // video
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform vec2 iChannelResolution[3];

#define Res  iResolution.xy
#define Res0 iChannelResolution[0].xy
#define Res1 iChannelResolution[1].xy

#define PI 3.14159265358979

vec4
getCol (vec2 pos)
{
  vec2 uv = pos / Res0;
  vec4 c1 = texture (iChannel0, uv);
  vec4 c2 = vec4 (.4);
  float d = clamp (dot (c1.xyz, vec3 (-0.5, 1.0, -0.5)), 0.0, 1.0);
  return mix (c1, c2, 1.8 * d);
}

vec4
getCol2 (vec2 pos)
{
  vec2 uv = pos / Res0;
  vec4 c1 = texture (iChannel0, uv);
  vec4 c2 = vec4 (1.5);
  float d = clamp (dot (c1.xyz,vec3 (-0.5, 1.0, -0.5)), 0.0, 1.0);
  return mix (c1, c2, 1.8 * d);
}

vec2
getGrad (vec2 pos,float delta)
{
  vec2 d = vec2 (delta, 0.0);
  return vec2 (dot((getCol(pos+d.xy)-getCol(pos-d.xy)).xyz,vec3(.333)),
               dot((getCol(pos+d.yx)-getCol(pos-d.yx)).xyz,vec3(.333)))/delta;
}

vec2
getGrad2 (vec2 pos,float delta)
{
  vec2 d = vec2 (delta, 0.0);
  return vec2 (dot((getCol2(pos+d.xy)-getCol2(pos-d.xy)).xyz,vec3(.333)),
               dot((getCol2(pos+d.yx)-getCol2(pos-d.yx)).xyz,vec3(.333)))/delta;
}

vec4
getRand (vec2 pos) 
{
  vec2 uv = pos / Res1;
  return texture (iChannel1, uv);
}

float
htPattern (vec2 pos)
{
  float p;
  float r = getRand (pos * 0.4 / 0.7 * 1.0).x;
  p = clamp ((pow (r + 0.3, 2.0) - 0.45), 0.0, 1.0);
  return p;
}

float
getVal (vec2 pos, float level)
{
  return length (getCol (pos).xyz) + 0.0001 * length (pos - 0.5 * Res0);
  return dot (getCol (pos).xyz, vec3 (0.333));
}

vec4
getBWDist (vec2 pos)
{
  return vec4 (smoothstep (0.9, 1.1, getVal (pos, 0.0) * 0.9 + htPattern (pos * 0.7)));
}

#define SampNum 24

#define N(a) (a.yx*vec2(1,-1))

void
main ()
{
  vec2 pos = ((gl_FragCoord.xy - Res.xy * 0.5) / Res.y * Res0.y) + Res0.xy * 0.5;
  vec2 pos2 = pos;
  vec2 pos3 = pos;
  vec2 pos4 = pos;
  vec2 pos0 = pos;
  vec3 col = vec3 (0.0);
  vec3 col2 = vec3 (0.0);
  float cnt = 0.0;
  float cnt2 = 0.0;

  for (int i = 0; i < 1 * SampNum; i++)
  {
    vec2 gr = getGrad (pos, 2.0) + 0.0001 * (getRand (pos ).xy - 0.5);
    vec2 gr2 = getGrad (pos2, 2.0) + 0.0001 * (getRand (pos2).xy - 0.5);

    vec2 gr3 = getGrad2 (pos3, 2.0) + 0.0001 * (getRand (pos3).xy - 0.5);
    vec2 gr4 = getGrad2 (pos4, 2.0) + 0.0001 * (getRand (pos4).xy - 0.5);

    float grl = clamp (10.0 * length (gr), 0.0, 1.0);
    float gr2l = clamp (10.0 * length (gr2), 0.0, 1.0);

    pos += 0.8 * normalize (N (gr));
    pos2 -= 0.8 * normalize (N (gr2));
    float fact = 1.0 - float(i) / float(SampNum);
    col += fact * mix (vec3 (1.2), getBWDist (pos).xyz * 2.0, grl);
    col += fact * mix (vec3 (1.2), getBWDist (pos2).xyz * 2.0, gr2l);

    pos3 += 0.25 * normalize (gr3) + 0.5 * (getRand (pos0 * 0.07).xy - 0.5);
    pos4 -= 0.5 * normalize (gr4) + 0.5 * (getRand (pos0 * 0.07).xy - 0.5);

    float f1 = 3.0 * fact;
    float f2 = 4.0 * (0.7 - fact);
    col2 += f1 * (getCol2 (pos3).xyz + 0.25 + 0.4 * getRand (pos3 * 1.0).xyz);
    col2 += f2 * (getCol2 (pos4).xyz + 0.25 + 0.4 * getRand (pos4 * 1.0).xyz);

    cnt2 += f1 + f2;
    cnt += fact;
  }
  col /= cnt * 2.5;
  col2 /= cnt2 * 1.65;

  col = clamp (clamp (col * 0.9 + 0.1, 0.0, 1.0) * col2, 0.0, 1.0);
  col = col * vec3 (0.93, 0.93, 0.85) * mix (texture (iChannel2, gl_FragCoord.xy / iResolution.xy).xyz, vec3 (1.2), 0.7) + 0.15 * getRand (pos0 * 2.5).x;

  float r = length ((gl_FragCoord.xy - iResolution.xy * 0.5) / iResolution.x);
  float vign = 1.0 - r * r * r * r;

  gl_FragColor = vec4 (col * vign, 1.0);
}
