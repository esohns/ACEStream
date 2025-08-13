#version 330 core

//in vec3 ourPosition;
in vec4 ourColor;
in vec2 ourTexCoord;

out vec4 FragColor;

// texture sampler
uniform sampler2D texture1;
uniform float     time;

void
main ()
{
  //FragColor = ourColor;
  //FragColor = texture (texture1, ourTexCoord);
  //FragColor = texture (texture1, ourTexCoord) * ourColor;

  float pct = abs (sin (time));
  FragColor = mix (texture (texture1, ourTexCoord), ourColor, pct);
}
