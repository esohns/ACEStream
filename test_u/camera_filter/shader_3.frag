
uniform sampler2D tex0;
uniform vec2 texelSize;
uniform vec2 canvasSize;
uniform vec2 mouse;
uniform float time;

const float Bayer2x2[4] = float[](0., 2., 3., 1.);
const float Bayer4x4[16] = float[](0., 8., 2., 10., 12., 4., 14., 6., 3., 11., 1., 9., 15., 7., 13., 5.);

float dither5(float t, vec2 coord)
{
  ivec2 g = ivec2(mod(coord, 2.));
  int i = g.x+g.y*2;
  float v = round(t*4.)/4.;
  return v>Bayer2x2[i]/4. ? 1. : 0.;
}

float dither17(float t, vec2 coord)
{
  ivec2 g = ivec2(mod(coord, 4.));
  int i = g.x+g.y*4;
  float v = round(t*16.)/16.;
  return v>Bayer4x4[i]/16. ? 1. : 0.;
}

float luminance(vec3 color)
{
  return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

in vec2 vTexCoord;

void main() {
  vec2 uv = vTexCoord;
  vec3 image = texture(tex0, uv).rgb;
  float g = luminance(image);
  vec3 col = vec3(0.);

  float it = floor(time*2.);
  if (mod(it, 4.) == 0.) col += image;
  if (mod(it, 4.) == 1.) col += g;
  if (mod(it, 4.) == 2.) col += dither5(g, gl_FragCoord.xy);
  if (mod(it, 4.) == 3.) col += dither17(g, gl_FragCoord.xy);

  gl_FragColor = vec4(col, 1.);
}
