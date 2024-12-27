#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 ourColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void
main ()
{
  gl_Position = projection * view * model * vec4 (aPos, 1.0f);
  ourColor = aColor;
// *IMPORTANT NOTE*: "...OpenGL stores textures starting at the bottom left
//                   pixel while images are usually stored starting with the top
//                   left pixel..." --> flip texture
  TexCoord = vec2 (aTexCoord.x, 1.0f - aTexCoord.y);
//  TexCoord = aTexCoord;
}
