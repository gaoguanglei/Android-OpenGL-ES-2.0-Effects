#version 150
uniform mat4 u_matViewProjection;

attribute vec3 a_position;

varying vec3 v_position;

void main(void)
{
	gl_Position = u_matViewProjection * vec4(a_position,1);
	v_position = a_position;

}