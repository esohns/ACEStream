#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

vec4
getCol (vec2 pos)
{
  vec2 uv=pos/iResolution.xy;
  vec4 c1=texture(iChannel0,uv);
  vec4 e=smoothstep(vec4(-0.05),vec4(-0.0),vec4(uv,vec2(1)-uv));
  c1=mix(vec4(1,1,1,0),c1,e.x*e.y*e.z*e.w);
  float d=clamp(dot(c1.xyz,vec3(-.5,1.,-.5)),0.0,1.0);
  vec4 c2=vec4(.7);
  return min(mix(c1,c2,1.8*d),.7);
}

vec4
getColHT (vec2 pos)
{
  return smoothstep(0.795,1.05,getCol(pos)*.8+.2+1.0);
}

float
getVal (vec2 pos)
{
  vec4 c = getCol(pos);
  return pow(dot(c.xyz,vec3(.333)),1.)*1.;
}

vec2
getGrad (vec2 pos, float eps)
{
  vec2 d=vec2(eps,0.);
  return vec2(getVal(pos+d.xy)-getVal(pos-d.xy),
              getVal(pos+d.yx)-getVal(pos-d.yx)
             )/eps/2.;
}

float
lum (vec3 c)
{
  return dot(c, vec3(0.3, 0.59, 0.11));
}

vec3
clipcolor (vec3 c)
{
  float l = lum(c);
  float n = min(min(c.r, c.g), c.b);
  float x = max(max(c.r, c.g), c.b);

  if (n < 0.0)
  {
    c.r = l + ((c.r - l) * l) / (l - n);
    c.g = l + ((c.g - l) * l) / (l - n);
    c.b = l + ((c.b - l) * l) / (l - n);
  }
  if (x > 1.25)
  {
    c.r = l + ((c.r - l) * (1.0 - l)) / (x - l);
    c.g = l + ((c.g - l) * (1.0 - l)) / (x - l);
    c.b = l + ((c.b - l) * (1.0 - l)) / (x - l);
  }

  return c;
}

vec3
setlum (vec3 c,  float l)
{
  float d = l - lum(c);
  c = c + vec3(d);
  return clipcolor(0.85*c);
}

#define AngleNum 3
#define SampNum 9
#define PI2 6.28318530717959

void
main ()
{
  vec2 pos = gl_FragCoord.xy;
  vec3 col = vec3(0);
  vec3 col2 = vec3(0);
  float sum=0.;

  for(int i=0;i<AngleNum;i++)
  {
    float ang=PI2/float(AngleNum)*(float(i)+0.8);
    vec2 v=vec2(cos(ang),sin(ang));
    for(int j=0;j<SampNum;j++)
    {
      vec2 dpos  = v.yx*vec2(1,-1)*float(j)*iResolution.y/920.;
      vec2 dpos2 = 5.0*( v.xy*float(j*j)/float(SampNum)*.5*iResolution.y/920.);
      vec2 g;
      float fact;
      float fact2;
      float s=3.5;

      vec2 pos2=pos+s*dpos+dpos2;

      g=getGrad(pos2,0.08);
      fact=dot(g,v)-.5*abs(dot(g,v.yx*vec2(1,-1)));
      fact2=dot(normalize(g+vec2(.0001)),v.yx*vec2(1,-1));

      fact=clamp(fact,0.,.05);
      fact2=abs(fact2);

      fact*=1.-float(j)/float(SampNum);
      col += fact;
      col2 += fact2;
      sum+=fact2;
    }
  }
  col/=float(SampNum*AngleNum)*0.65/sqrt(iResolution.y);
  col2/=sum;
  col.x*=1.6;
  col.x=1.-col.x;
  col.x*=col.x*col.x;

  vec2 s=sin(pos.xy*.1/sqrt(iResolution.y/720.));
  vec3 karo=vec3(1);
  karo-=.75755*vec3(.25,.1,.1)*dot(exp(-s*s*80.),vec2(1.));
  float r=length(pos-iResolution.xy*.5)/iResolution.x;
  float vign=1.-r*r*r;
  gl_FragColor = vec4(vec3(col.x*col2*karo*vign),1.0);
  vec4 origCol = texture(iChannel0, gl_FragCoord.xy/iResolution.xy);
  vec4 overlayColor = vec4(0.3755,0.05,0.0,0.0)*origCol;

  gl_FragColor = vec4( setlum(1.25*overlayColor.rgb, lum(gl_FragColor.rgb)) * 1.0, 1.0);
  gl_FragColor.rgb -= 0.75 - clamp (origCol.r + origCol.g + origCol.b , 0.0 , 0.75);
}
