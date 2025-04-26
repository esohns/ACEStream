#version 130

// shader_32_common.glsl
const int NumPasses = 13;
// shader_32_common.glsl

uniform vec2 iResolution;
uniform float iTime;
uniform int iFrame;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;

#define iPassIndex (iFrame%NumPasses)

#define Res0 vec2(textureSize(iChannel0,0))
#define Res1 vec2(textureSize(iChannel1,0))
#define Res2 vec2(textureSize(iChannel2,0))

vec4
getCol (vec2 pos)
{
  vec2 tres = Res0;
  vec2 tpos = (pos-.5*Res1)*min(tres.y/Res1.y,tres.x/Res1.x);
  vec2 uv = (tpos+tres*.5)/tres;
  vec4 col=texture(iChannel0,uv);
  vec4 bg=texture(iChannel2,((uv-.5)*.9+.5)+.05*sin(.1*iTime*vec2(2,3)),3.7+log2(Res1.x/600.));
  col=mix(col,bg,dot(col.xyz,vec3(-.8,1.6,-.8)));
  return col;
}

void
main ()
{
  if (iPassIndex==0)
    gl_FragColor=getCol(gl_FragCoord.xy);
  else
    gl_FragColor=texture(iChannel1,gl_FragCoord.xy/iResolution.xy);
}
