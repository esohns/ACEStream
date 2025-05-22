#version 130

uniform vec2 iResolution;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

#define C_RED vec4(1.0, 0.0, 0.0, 1.0)
#define C_YELLOW vec4(1.0, 1.0, 0.0, 1.0)
#define C_BLUE vec4(0.0, 0.0, 1.0, 1.0)

void
main ()
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy;
  vec4 c = texture(iChannel0, uv); 
  float luminance = 0.299 * c.r + 0.587 * c.g + 0.114 * c.b;
  float THRESHOLD = (length(iMouse.xy) < 1e-2) ? 0.5 : iMouse.x / iResolution.x;
  gl_FragColor = (luminance < THRESHOLD) ? mix(C_BLUE, C_YELLOW, luminance * 2.0 ) : mix(C_YELLOW, C_RED, (luminance - 0.5) * 2.0);
  gl_FragColor.rgb *= 0.1 + 0.25 + 0.75 * pow(16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y), 0.15);
}
