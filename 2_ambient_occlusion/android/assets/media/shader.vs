uniform mat4 u_matViewProjection;


attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texCoord;


varying vec3 pos;
varying vec2 texCoord;

void main(void)
{
  gl_Position = u_matViewProjection * vec4(a_position,1);
 pos = a_normal;
   texCoord = a_texCoord;
}