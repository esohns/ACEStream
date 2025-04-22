#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

float MAXD=1000.0;
float perspective=1.25,zoom=.79;
vec3 cameraLocation=vec3(0.0, -10.5, 0.0);
vec3 light=vec3(-0.1,-0.1, -1.);
float rotX=1.45;
float dist=1.5;
vec3 center=vec3(0.0,0.0,0.0);
vec3 shift=vec3(64./2.,0.,36./1.5);

vec3
rotateX (vec3 p, float angle)
{
  float s = sin(angle);
  float c = cos(angle);
  return vec3( p.x, c * p.y + s * p.z, -s * p.y + c * p.z);
}

float
map (vec3 p)
{
  p=rotateX(p,rotX);
  p +=shift;
  if (p.x>-0.5 && p.x<64.5 && p.z>-0.5 && p.z<48.5 && p.y>24.5 && p.y<87.5)
  {
    vec4 tex=textureLod(iChannel0,p.xz/vec2(64.0,48.0).xy,0.0);

    float tl=max(.25,tex.r*1.5);
    float div=2.;
    p.x = mod( p.x + dist/div, dist ) - dist/div;
    float dist2=2.75;
    p.y = (mod( p.y + dist2/div, dist2 ) - dist2/div)-1.5;
    p.z = mod( p.z + dist/div, dist ) - dist/div;

    return length( center - p ) - tl;
  }

  return float(1.);
}

vec3
calc_normal (vec3 v)
{
  float e=0.0001;
  vec3 n=vec3(
  map(vec3(v.x+e,v.y,v.z))-map(vec3(v.x-e,v.y,v.z)),
  map(vec3(v.x,v.y+e,v.z))-map(vec3(v.x,v.y-e,v.z)),
  map(vec3(v.x,v.y,v.z+e))-map(vec3(v.x,v.y,v.z-e)));
  return normalize(n);
}

float
ao (vec3 p, vec3 n, float d)
{
  float o=1.0,ii=5.0;
  for(int i=0;i<5;i++)
  {
    vec3 tmpV=p+n*(ii*d);
    float tmp=map(tmpV);
    if(tmp<0.0)
      tmp=0.0;
    o-=(ii*d-tmp)/pow(2.0,ii);
    ii=ii-1.0;
  }
  return o;
}

void
main ()
{
  float x,y,off=0.0;
  vec4 col;
  float rx,ry,d;
  int steps=0;
  vec3 ray,direction;

  vec2 p = 2.0*gl_FragCoord.xy;
  p.x/= iResolution.x;
  p.y/= iResolution.x;
  p-=1.0;
  p.y=-p.y;
  p=p/zoom;
  ray=vec3(p.x,p.y,0);
  ray=ray+cameraLocation;
  direction=vec3(p.x*perspective,p.y*perspective,1.0);
  direction=normalize(direction);
  col=vec4(0.25+(p.y+0.5)/3.0,0.25+(p.y+0.5)/3.0,0.33+(p.y+0.5)/3.0,1.0);

  for(int i=0;i<150;i++)
  {
    d=map(ray);
    if(d>=MAXD)
      break;
    if(d<0.0001)
    {
      vec3 n=calc_normal(ray);
      float normlight=0.25*max(0.,dot(light,n));
      float aolight=ao(ray,n,0.25);
      if(normlight<0.0)
        normlight=0.0;
      float ambient=0.75;
      float c=(normlight+ambient)*aolight;
      col=vec4(c,c,c,1.0);
      vec3 p2=rotateX(ray,rotX);
      p2 +=shift;
      vec4 tex=texture(iChannel0,p2.xz/vec2(64.0,48.0).xy);
      col *=tex /(1.+map(ray));
      break;
    }

    ray+=direction*d;
  }

  gl_FragColor = col;
}
