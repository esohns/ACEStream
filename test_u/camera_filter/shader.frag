precision mediump float;

varying vec2 vTexCoord;

uniform sampler2D tex0;
uniform vec2 texSize;

const vec3 W = vec3 (0.2125, 0.7154, 0.0721);
//const float levels = 8.0;

void main ()
{
  vec2 onePixel = vec2 (1.0, 1.0) / texSize;

  float brightnessTL = dot (texture2D (tex0, vTexCoord + onePixel * vec2 (-1, -1)).rgb, W);
  float brightnessT  = dot (texture2D (tex0, vTexCoord + onePixel * vec2 ( 0, -1)).rgb, W);
  float brightnessTR = dot (texture2D (tex0, vTexCoord + onePixel * vec2 ( 1, -1)).rgb, W);
  float brightnessBL = dot (texture2D (tex0, vTexCoord + onePixel * vec2 (-1,  1)).rgb, W);
  float brightnessB  = dot (texture2D (tex0, vTexCoord + onePixel * vec2 ( 0,  1)).rgb, W);
  float brightnessBR = dot (texture2D (tex0, vTexCoord + onePixel * vec2 ( 1,  1)).rgb, W);

  float gradient = brightnessTL * -1.0 +
                   brightnessT  * -2.0 +
                   brightnessTR * -1.0 +
                   brightnessBL *  1.0 +
                   brightnessB  *  2.0 +
                   brightnessBR *  1.0;

  float normalizedGradient = clamp (abs (gradient) * 5.0, 0.0, 1.0);

  vec4 edgeColor = vec4 (0.0, 0.8, 1.0, 1.0); // Set the desired color for the edges (high sky blue)

  // *DEBUG*: map texture
  //gl_FragColor = vec4 (texture2D (tex0, vTexCoord).rgb, 1.0);
  
  // Mix the edge color with the black background based on the edge intensity
  gl_FragColor = mix (vec4 (0.0), edgeColor, normalizedGradient);

  // alternative: cartoon style
//  vec4 color = vec4 (texture2D (tex0, vTexCoord).rgb, 1.0);
//  color.rgb = floor (color.rgb * levels) / levels;
//  gl_FragColor = color;
}
