#version 130

uniform vec2 iResolution;
uniform sampler2D iChannel0;

void
main ()
{
  vec4 O;
  vec2 U = gl_FragCoord.xy;

  O = texture(iChannel0, U / iResolution.xy);

  //O = pow(O,vec4(1./3.));
  //O = vec4(O.x);      // B&W
  //O = vec4(pow(O.x,.25));
  //O = vec4(O.x>.7);  // binarisation B&W - base morpho maths
  //O = step(.5,O);    // binarisation col

  gl_FragColor = O;
}
