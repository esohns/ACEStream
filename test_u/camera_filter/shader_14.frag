#version 130

uniform vec2 iResolution;
uniform float iTime;
uniform sampler2D iChannel0;
uniform sampler2D iChannel2;

#define A 6

float hash (vec2 p) { return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453); }

float
arrivingParticle (vec2 coord, out vec4 partData)
{
  for (int i=-A; i<A; i++)
  {
    for (int j=-A; j<A; j++)
    {
      vec2 arrCoord = coord + vec2(i,j);
      vec4 data = texture(iChannel0, arrCoord/iResolution.xy);

      if (dot(data,data)<.1)
        continue;

      vec2 nextCoord = data.xy + data.zw;

      vec2 offset = abs(coord - nextCoord);
      if (offset.x<.5 && offset.y<.5)
      {
        partData = data;
        return 1.;
      }
    }
  }

  return 0.;
}

void
main ()
{
  vec2 uv = gl_FragCoord.xy/iResolution.xy;
  if (gl_FragCoord.y>iResolution.y-3.)
  {
    gl_FragColor = vec4(gl_FragCoord.xy,(hash(uv+iTime)-.8)*4.,-6.+hash(uv));
    return;
  }

  vec4 partData;
  float p = arrivingParticle(gl_FragCoord.xy, partData);
  if (p<1.)
  {
    gl_FragColor = vec4(0.);
    return;
  }

  float vel=max(0.,1.-length(texture(iChannel2,gl_FragCoord.xy/iResolution.xy).rb)*.95);
  partData.xy+=partData.zw*vel;

  gl_FragColor = partData;
}
