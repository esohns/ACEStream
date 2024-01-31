attribute vec3 aPosition;
attribute vec2 aTexCoord;

varying vec2 vTexCoord;

void main ()
{
//  vTexCoord = vec2 (aTexCoord.x, 1.0 - aTexCoord.y);
  vTexCoord = vec2 (aPosition.x, 1.0 - aPosition.y);
//  vTexCoord = vec2(gl_TextureMatrix[0] * vec4(aPosition.xy, 1, 1));
//  vTexCoord = vec2 ((aPosition.x + 1.0) / 2.0, (1.0 - aPosition.y) / 2.0);

  vec4 positionVec4 = vec4 (aPosition, 1.0);
  positionVec4.xy = positionVec4.xy * 2.0 - 1.0;
  gl_Position = positionVec4;
}
