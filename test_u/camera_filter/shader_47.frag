#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;

void
main ()
{
  vec4 o = vec4(0.);
  vec2 U = gl_FragCoord.xy;

  float r=.1, t=iTime, H = iResolution.y;
  U /= H;
  vec2 P = .5+.5*vec2(cos(t),sin(t*.7)), fU;  
  U*=.5; P*=.5;

  //o.b = .25;

  for (int i=0; i<7; i++)
  {
    fU = min(U,1.-U);
    if (min(fU.x,fU.y) < 3.*r/H)
    {
//      o--;
      break;
    }
    if (length(P-.5) - r > .7)
      break;

    fU = step(.5,U);
    U = 2.*U - fU;
    P = 2.*P - fU;  r *= 2.;

    o = texture(iChannel0,U);
  }

  o.gb *= smoothstep(.9,1.,length(P-U)/r);

  gl_FragColor = o;
}
