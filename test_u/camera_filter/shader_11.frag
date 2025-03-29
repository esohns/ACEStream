#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform int iFrame;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

#define SUBDIVIDE
#define SPARKLES
#define GRAYSCALE
#define FAR 20.

float objID;

mat2 rot2 (float a) { float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }

float hash21 (vec2 p) { return fract(sin(dot(p, vec2(27.609, 57.583)))*43758.5453); }

vec3
getTex (vec2 p)
{
  p *= vec2(iResolution.y/iResolution.x, 1);
  vec3 tx = texture(iChannel0, fract(p/2. - .5)).xyz;
  return tx*tx;
}

float hm (vec2 p) { return dot(getTex(p), vec3(.299, .587, .114)); }

float
opExtrusion (float sdf, float pz, float h)
{
  vec2 w = vec2( sdf, abs(pz) - h );
  return min(max(w.x, w.y), 0.) + length(max(w, 0.));

  const float sf = .025;
  w = vec2( sdf, abs(pz) - h) + sf;
  return min(max(w.x, w.y), 0.) + length(max(w, 0.)) - sf;
}

float sBoxS (vec2 p, vec2 b, float sf)
{
  return length(max(abs(p) - b + sf, 0.)) - sf;
}

vec4
blocks (vec3 q3)
{
  const float scale = 1./16.;

  const vec2 l = vec2(scale);
  const vec2 s = l*2.;

  float d = 1e5;
  vec2 p, ip;

  vec2 id = vec2(0);
  vec2 cntr = vec2(0);

  vec2[4] ps4 = vec2[4](vec2(-l.x, l.y), l, -l, vec2(l.x, -l.y));

  float boxID = 0.;

  for(int i = 0; i<4; i++)
  {
    cntr = ps4[i]/2.;

    p = q3.xy - cntr;
    ip = floor(p/s) + .5;
    p -= (ip)*s;

    vec2 idi = ip*s + cntr;

    float h = hm(idi);
#ifndef SUBDIVIDE
    h = floor(h*15.999)/15.*.15;
#endif

#ifdef SUBDIVIDE
    vec4 h4;
    int sub = 0;
    for(int j = 0; j<4; j++)
    {
      h4[j] = hm(idi + ps4[j]/4.);
      if(abs(h4[j] - h)>1./15.) sub = 1;
    }

    h = floor(h*15.999)/15.*.15;
    h4 = floor(h4*15.999)/15.*.15;

    if(sub==1)
    {
      vec4 d4, di4;

      for(int j = 0; j<4; j++)
      {
        d4[j] = sBoxS(p - ps4[j]/4., l/4. - .05*scale, .005);
        di4[j] = opExtrusion(d4[j], (q3.z + h4[j]), h4[j]);
                
        if(di4[j]<d)
        {
          d = di4[j];
          id = idi + ps4[j]/4.;
        }
      }
    }
    else
    {
#endif
      float di2D = sBoxS(p, l/2. - .05*scale, .015);

      float di = opExtrusion(di2D, (q3.z + h), h);

      if(di<d)
      {
        d = di;
        id = idi;
      }
#ifdef SUBDIVIDE    
    }
#endif
  }

  return vec4(d, id, boxID);
}

vec2 gID;
float
map (vec3 p)
{
  float fl = -p.z + .1;

  vec4 d4 = blocks(p);
  gID = d4.yz;

  objID = fl<d4.x? 1. : 0.;

  return min(fl, d4.x);
}

float
trace (vec3 ro, vec3 rd)
{
  float t = 0., d;

  for(int i = min(iFrame, 0); i<64; i++)
  {
    d = map(ro + rd*t);
    if(abs(d)<.001 || t>FAR)
      break;

    t += d*.7; 
  }

  return min(t, FAR);
}

vec3
getNormal (vec3 p, float t)
{
  const vec2 e = vec2(.001, 0);
  return normalize(vec3(map(p + e.xyy) - map(p - e.xyy), map(p + e.yxy) - map(p - e.yxy),	map(p + e.yyx) - map(p - e.yyx)));
}

float
softShadow (vec3 ro, vec3 lp, vec3 n, float k)
{
  const int maxIterationsShad = 24; 

  ro += n*.0015;
  vec3 rd = lp - ro; // Unnormalized direction ray.

  float shade = 1.;
  float t = 0.;
  float end = max(length(rd), .0001);
  rd /= end;

  for (int i = min(iFrame, 0); i<maxIterationsShad; i++)
  {
    float d = map(ro + rd*t);
    shade = min(shade, k*d/t);
    t += clamp(d, .01, .25); 

    if (d<0. || t>end)
      break;
  }

  return max(shade, 0.); 
}

float
calcAO (vec3 p, vec3 n)
{
  float sca = 3., occ = 0.;
  for( int i = 0; i<5; i++ )
  {
    float hr = float(i + 1)*.15/5.;
    float d = map(p + n*hr);
    occ += (hr - d)*sca;
    sca *= .7;
  }

  return clamp(1. - occ, 0., 1.);  
}

void
main ()
{
  vec2 uv = (gl_FragCoord.xy - iResolution.xy*.5)/iResolution.y;

  vec3 lk = vec3(0, 0, 0);//vec3(0, -.25, iTime);
  vec3 ro = lk + vec3(-.5*.3*cos(iTime/2.), -.5*.2*sin(iTime/2.), -2);

  vec3 lp = ro + vec3(1.5, 2, -1);

  float FOV = 1.;
  vec3 fwd = normalize(lk-ro);
  vec3 rgt = normalize(vec3(fwd.z, 0., -fwd.x )); 
  vec3 up = cross(fwd, rgt); 

  vec3 rd = normalize(fwd + FOV*uv.x*rgt + FOV*uv.y*up);
//  rd.xy *= rot2( sin(iTime)/32. );

/*  vec2 ms = vec2(0);
  if (iMouse.z > 1.0) ms = (iMouse.xy - iResolution.xy*.5)/iResolution.xy;
  vec2 a = sin(vec2(1.5707963, 0) - ms.x);
  mat2 rM = mat2(a, -a.y, a.x);
  rd.xz = rd.xz*rM; 
  a = sin(vec2(1.5707963, 0) - ms.y);
  rM = mat2(a, -a.y, a.x);
  rd.yz = rd.yz*rM;
*/

  float t = trace(ro, rd);

  vec2 svGID = gID;

  float svObjID = objID;

  vec3 col = vec3(0);

  if(t < FAR)
  {
    vec3 sp = ro + rd*t;
    vec3 sn = getNormal(sp, t);

    vec3 texCol;

    if(svObjID<.5)
    {
      vec3 tx = getTex(svGID);
#ifdef GRAYSCALE
      texCol = vec3(1)*dot(tx, vec3(.299, .587, .114));
#else 
      texCol = tx;
#endif

#ifdef SPARKLES
      float rnd = fract(sin(dot((svGID), vec2(141.13, 289.97)))*43758.5453);
      float rnd2 = fract(sin(dot((svGID + .037), vec2(141.13, 289.97)))*43758.5453);
      rnd = smoothstep(.9, .95, cos(rnd*6.283 + iTime*2.)*.5 + .5);
      vec3 rndCol = (.5 + .45*cos(6.2831*mix(0., .3, rnd2) + vec3(0, 1, 2)/1.1));
      rndCol = mix(rndCol, rndCol.xzy, uv.y*.75 + .5);
      rndCol = mix(vec3(1), rndCol*50., rnd*smoothstep(1. - (1./1./15. + .001), 1., 1. - texCol.x));

      texCol *= rndCol;
#endif

      texCol = smoothstep(0., 1., texCol);
    }
    else
    {
      texCol = vec3(0);
    }

    vec3 ld = lp - sp;

    float lDist = max(length(ld), .001);

    ld /= lDist;

    float sh = softShadow(sp, lp, sn, 8.);
    float ao = calcAO(sp, sn);
    sh = min(sh + ao*.25, 1.);

    float atten = 1./(1. + lDist*.05);

    float diff = max( dot(sn, ld), 0.);

    float spec = pow(max(dot(reflect(ld, sn), rd ), 0.), 16.); 

    float fre = pow(clamp(dot(sn, rd) + 1., 0., 1.), 2.);

    col = texCol*(diff + ao*.3 + vec3(.25, .5, 1)*diff*fre*16. + vec3(1, .5, .2)*spec*2.);

    col *= ao*sh*atten;
  }

  gl_FragColor = vec4(sqrt(max(col, 0.)), 1);
}
