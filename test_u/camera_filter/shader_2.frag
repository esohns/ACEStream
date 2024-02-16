#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
//uniform vec2 u_mouse;
uniform float u_time;
uniform sampler2D u_img;

float random (in vec2 st)
{
  return fract (sin (dot (st.xy, vec2 (12.9898,78.233))) * 43758.5453123);
}

float noise (vec2 st)
{
  vec2 i = floor (st);
  vec2 f = fract (st);
  vec2 u = f*f*(3.0-2.0*f);
  return mix( mix( random( i + vec2(0.0,0.0) ),
                    random( i + vec2(1.0,0.0) ), u.x),
              mix( random( i + vec2(0.0,1.0) ),
                    random( i + vec2(1.0,1.0) ), u.x), u.y);
}

mat2 rotate2d (float angle)
{
  return mat2 (cos (angle), -sin (angle),
               sin (angle),  cos (angle));
}

float lines (in vec2 pos, float b)
{
  float scale = 6.;
  pos *= scale;
  return smoothstep (0.0,
                     0.5+b*0.5,
                     abs ((sin (pos.x*3.1415)+b*2.0))*0.5);
}

float grid (in vec2 uv)
{
  float scale = 2.0;
  uv *= scale;
  uv = fract (uv);
  float o = uv.x >0.99 && uv.x <= 1. || uv.y >0.99 && uv.y <= 1. ? 1.0 : 0.0;
  return o;
}

void main ()
{
  vec2 st = gl_FragCoord.xy/u_resolution.xy;
  st.y *= u_resolution.y/u_resolution.x;
  st = vec2(st.x,1.-st.y);//for p5
  st.x += sin(st.y*  3.14*4. +  u_time*.5)*(0.1 * sin(u_time*0.3));
		
  vec2 pos = st;
  pos = rotate2d( noise(pos + vec2(sin(u_time*0.2)))*0.5) * (pos);// Add noise
	pos = fract(pos);
  float g = grid(pos);
    
  // Draw lines
	vec3 color = texture2D(u_img,pos).rgb;
  gl_FragColor = vec4(color,1.0);
}
