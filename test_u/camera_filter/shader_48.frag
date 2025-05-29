#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

void
main ()
{
  float sum = 0.;
  for( float i = 0. ; i < gl_FragCoord.x ; i++)
    sum += texture(iChannel0, vec2(i, gl_FragCoord.y) / iResolution.xy).g / iResolution.x;

  float lum = texture(iChannel0, gl_FragCoord.xy / iResolution.xy).g;

  gl_FragColor = vec4(sum, lum, 0, 1);
}
