uniform sampler2D videoTexture;
uniform float brightness;
uniform float contrast;

varying vec2 vTexCoord;

void
main ()
{
  vec4 texColor = texture2D (videoTexture, vTexCoord);

  // Adjust brightness
  vec3 adjustedColor = texColor.rgb * brightness;

  // Adjust contrast
  adjustedColor = (adjustedColor - 0.5) * contrast + 0.5;

  gl_FragColor = vec4 (adjustedColor, texColor.a);
}
