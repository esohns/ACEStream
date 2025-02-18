// shader_6_common.glsl
#define size iResolution.xy
#define pixel(a, p) texture(a, p/vec2(textureSize(a,0)))
#define texel(a, p) texelFetch(a, ivec2(p), 0)
#define ch0 iChannel0
#define ch1 iChannel1
#define ch2 iChannel2
#define ch3 iChannel3
#define PI 3.14159265

#define dt 0.4
#define prop 0.5

ivec2 N;
int tot_n;

float hash11(float p)
{
    p = fract(p * 15.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p) - 0.5;
}

float hash12(vec2 p)
{
  vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}


vec2 hash21(float p)
{
  vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
  p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx+p3.yz)*p3.zy);

}

vec2 hash22(vec2 p)
{
  vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

const int k = 1664525;  

ivec4 hash( ivec4 x )
{
    x = ((x>>8)^x.wxyz)*k;
    x = ((x>>8)^x.wxyz)*k;
    x = ((x>>8)^x.wxyz)*k;
    x = ((x>>8)^x.wxyz)*k;
    return ivec4(x);
}

ivec2 i2xy(int id)
{
    return ivec2(id%N.x, id/N.x);
}

int xy2i(ivec2 p)
{
    return p.x + p.y*N.x;
}

ivec2 cross_distribution(int i)
{
    return (1<<(i/4)) * ivec2( ((i&2)/2)^1, (i&2)/2 ) * ( 2*(i%2) - 1 );
}

float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}
// shader_6_common.glsl

uniform vec2 iResolution;
uniform float iTime;
uniform int iFrame;
uniform vec4 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;

//particle buffer

int cid;

ivec4 get(int id)
{
    return ivec4(texel(ch0, i2xy(id)));
}

vec4 getParticle(int id)
{
    return texel(ch1, i2xy(id));
}

float F(float d)
{
    return (0.15*exp(-0.1*d) - 2.*exp(-0.2*d));
}

float imageV(vec2 p)
{
    return 1.-2.*texture(ch2, vec2(1., 1.)*p/size).x;
}

vec2 imageF(vec2 p)
{
    vec3 d = vec3(-1,0,1);
    return vec2(imageV(p+d.zy) - imageV(p+d.xy), imageV(p+d.yz) - imageV(p+d.yx));
}

vec2 Fv(vec4 p0, int pid)
{
    if(pid < 0 || pid >= tot_n || pid == cid) return vec2(0); 
    vec4 p1 = getParticle(pid);
    float d= distance(p0.xy, p1.xy);
    vec2 dv = (p1.zw - p0.zw);
    float dotv = dot(normalize(p1.xy-p0.xy), normalize(dv)); //divergence correction
    vec2 antidivergence = 0.*dv*abs(dotv)*exp(-0.5*d);
    vec2 viscosity = 0.25*dv*exp(-0.1*d);
    vec2 pressure = normalize(p1.xy-p0.xy)*F(d);
    return viscosity + pressure + antidivergence;
}

float irad;

vec2 Fspring(vec4 p0, int pid)
{
    if(pid < 0 || pid >= tot_n || pid == cid) return vec2(0); 
    vec4 p1 = getParticle(pid);
    vec2 interaction = normalize(p1.xy-p0.xy)*(distance(p1.xy,p0.xy)- 2.*PI*irad/float(tot_n) - 4.*tanh(0.1*iTime));
    return interaction;
}

void
main ()
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    N = ivec2(prop*iResolution.xy);
    tot_n = N.x*N.y;
    if(p.x < N.x && p.y < N.y)
    {
        irad = 0.3*size.y;
        vec2 pos = floor(gl_FragCoord.xy);
        //this pixel value
        gl_FragColor = texel(ch1, pos);
        int id = xy2i(p);
        cid = id;
        
        //this pixel value
        if(iFrame<10)
        {
            float t = 2.*PI*float(id)/float(tot_n);
            gl_FragColor.xy = size*hash22(3.14159*pos);
            gl_FragColor.zw = 1.*(hash22(3.14159*pos) - 0.5);
          return;
        }
        
        //neighbors
      ivec4 cp = get(id);
      
        vec2 F = Fv(gl_FragColor, cp.x) +
               Fv(gl_FragColor, cp.y) +
               Fv(gl_FragColor, cp.z) +
                 Fv(gl_FragColor, cp.w) +
               -20.*imageF(gl_FragColor.xy);
        
        if(iMouse.z > 0.) 
        {
            float d = distance(iMouse.xy, gl_FragColor.xy);
            F += 2.*normalize(iMouse.xy - gl_FragColor.xy)/(sqrt(d)+2.);
        }
        
        gl_FragColor.zw = 15.*tanh((F*dt + gl_FragColor.zw)/15.) ;
        gl_FragColor.xy += gl_FragColor.zw*dt;
        
        //border conditions
        if(size.x - gl_FragColor.x < 2.) gl_FragColor.z = -abs(gl_FragColor.z);
        if(gl_FragColor.x < 2.) gl_FragColor.z = abs(gl_FragColor.z);
        if(size.y - gl_FragColor.y < 2.) gl_FragColor.w = -abs(gl_FragColor.w);
        if(gl_FragColor.y < 2.) gl_FragColor.w = abs(gl_FragColor.w);
 
        
    }
    else discard;
}
