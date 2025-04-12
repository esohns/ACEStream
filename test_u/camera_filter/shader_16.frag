#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

#define GAMMA 0.65
#define REGIONS 5.
#define LINES 0.5
#define BASE 2.5
#define GREEN_BIAS 0.9

vec3
OutlineWhyCantIPassASampler (vec2 fragCoord)
{
  vec2 uv = fragCoord.xy / iResolution.xy;
  uv.y = 1.-uv.y;
  vec4 lines= vec4(0.30, 0.59, 0.11, 1.0);

  lines.rgb = lines.rgb * LINES*1.5;

  float s11 = dot(texture(iChannel1, uv + vec2(-1.0 / iResolution.x, -1.0 / iResolution.y)), lines);   // LEFT
  float s12 = dot(texture(iChannel1, uv + vec2(0, -1.0 / iResolution.y)), lines);             // MIDDLE
  float s13 = dot(texture(iChannel1, uv + vec2(1.0 / iResolution.x, -1.0 / iResolution.y)), lines);    // RIGHT

  float s21 = dot(texture(iChannel1, uv + vec2(-1.0 / iResolution.x, 0.0)), lines);                // LEFT
  // Omit center
  float s23 = dot(texture(iChannel1, uv + vec2(1.0 / iResolution.x, 0.0)), lines);                // RIGHT

  float s31 = dot(texture(iChannel1, uv + vec2(-1.0 / iResolution.x, 1.0 / iResolution.y)), lines);    // LEFT
  float s32 = dot(texture(iChannel1, uv + vec2(0, 1.0 / iResolution.y)), lines);              // MIDDLE
  float s33 = dot(texture(iChannel1, uv + vec2(1.0 / iResolution.x, 1.0 / iResolution.y)), lines); // RIGHT

  float t1 = s13 + s33 + (2.0 * s23) - s11 - (2.0 * s21) - s31;
  float t2 = s31 + (2.0 * s32) + s33 - s11 - (2.0 * s12) - s13;

  vec3 col;

  if (((t1 * t1) + (t2* t2)) > 0.04) 
    col = vec3(-1.,-1.,-1.);
  else
    col = vec3(0.,0.,0.);
 
  return col;
}

vec3
RecolorForeground (vec3 color)
{
  if(color.g > (color.r + color.b)*GREEN_BIAS)
  {
    color.rgb = vec3(0.,0.,0.);
  }

  color.rgb = 0.2126*color.rrr + 0.7152*color.ggg + 0.0722*color.bbb;

  if(color.r > 0.95)
  {
  }
  else if(color .r > 0.75)
  {
    color.r *= 0.9;
  }
  else if(color.r > 0.5)
  {
    color.r *= 0.7;
    color.g *=0.9;
  }
  else if (color.r > 0.25)
  {
    color.r *=0.5;
    color.g *=0.75;
  }
  else
  {
    color.r *= 0.25;
    color.g *= 0.5;
  }

  return color;
}

vec3
Posterize (vec3 color)
{
  color = pow(color, vec3(GAMMA, GAMMA, GAMMA));
  color = floor(color * REGIONS)/REGIONS;
  color = pow(color, vec3(1.0/GAMMA));
  return color.rgb;
}

vec3
ReplaceBackground (vec3 color,vec2 uv,vec2 fragCoord)
{
  if((color.g > color.r && color.g > color.b) && color.g > GREEN_BIAS)
  {
    color.r = texture(iChannel1,vec2(uv.x,1.-uv.y)).r-0.6;
    color.r = (0.5-0.5*uv.x+uv.y)*0.75 +color.r;
    color = Posterize(color.rrr);
    color.b = 0.;
    color.g = 0.;
    color += OutlineWhyCantIPassASampler(fragCoord);
  }
  else
    color = vec3(0.,0.,0.);	

  return color;
}

vec3
Outline (vec2 uv)
{
  vec4 lines= vec4(0.30, 0.59, 0.11, 1.0);

  lines.rgb = lines.rgb * LINES;
  if(iResolution.x < 300.)
  {
    lines /= 4.0;	// improves thumbnail look
  }
  else if(iResolution.x > 1000.)
  {
    lines *= 1.5;
  }

  float s11 = dot(texture(iChannel0, uv + vec2(-1.0 / iResolution.x, -1.0 / iResolution.y)), lines);   // LEFT
  float s12 = dot(texture(iChannel0, uv + vec2(0, -1.0 / iResolution.y)), lines);             // MIDDLE
  float s13 = dot(texture(iChannel0, uv + vec2(1.0 / iResolution.x, -1.0 / iResolution.y)), lines);    // RIGHT

  float s21 = dot(texture(iChannel0, uv + vec2(-1.0 / iResolution.x, 0.0)), lines);                // LEFT
  // Omit center
  float s23 = dot(texture(iChannel0, uv + vec2( 1.0 / iResolution.x, 0.0)), lines);                // RIGHT

  float s31 = dot(texture(iChannel0, uv + vec2(-1.0 / iResolution.x, 1.0 / iResolution.y)), lines);    // LEFT
  float s32 = dot(texture(iChannel0, uv + vec2(0, 1.0 / iResolution.y)), lines);              // MIDDLE
  float s33 = dot(texture(iChannel0, uv + vec2(1.0 / iResolution.x, 1.0 / iResolution.y)), lines); // RIGHT

  float t1 = s13 + s33 + (2.0 * s23) - s11 - (2.0 * s21) - s31;
  float t2 = s31 + (2.0 * s32) + s33 - s11 - (2.0 * s12) - s13;

  vec3 col;

  if (((t1 * t1) + (t2* t2)) > 0.04) 
    col = vec3(-1.,-1.,-1.);
  else
    col = vec3(0.,0.,0.);

  return col;
}

void
main ()
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy;
  //uv.y = 1.0-uv.y;
  vec3 color = normalize(texture(iChannel0,uv)).rgb*BASE;	
  color = Posterize(color);
  vec3 background = ReplaceBackground(color,uv,gl_FragCoord.xy);
  color.rgb += Outline(uv);
  color = RecolorForeground(color)+ background;
  gl_FragColor = vec4(color,1.);
}
