#version 130

uniform vec2 iResolution;
uniform int iFrame;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

#define Xnum 10
#define Ynum 10

void
main ()
{
  vec2 uv0 = gl_FragCoord.xy/iResolution.xy;
  int fr = int(uv0.x*float(Xnum))+int(uv0.y*float(Xnum))*Ynum;
  if(fr!=int(mod(float(iFrame),float(Xnum*Ynum)))) 
    gl_FragColor = texture(iChannel1,uv0);
  else
    gl_FragColor = texture(iChannel0,fract(uv0*vec2(Xnum,Ynum)));

  if(iFrame<5)
    gl_FragColor = texture(iChannel0,fract(uv0*vec2(Xnum,Ynum)));
}
