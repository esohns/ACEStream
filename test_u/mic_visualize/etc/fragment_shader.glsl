#version 330 core

in vec4 ourColor;
in vec2 TexCoord;

out vec4 FragColor;

// texture sampler
uniform sampler2D texture1;
uniform float     time;

void
main ()
{
  //FragColor = ourColor;
  //FragColor = texture (texture1, TexCoord);
  //FragColor = texture (texture1, TexCoord) * ourColor;

  float pct = abs (sin (time));
  FragColor = mix (texture (texture1, TexCoord), ourColor, pct);
}
