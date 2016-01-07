#version 150
uniform mat4 view_proj_matrix;
uniform mat4 view_matrix;

uniform float localTime;

float bounciness = 0.55;
float hardness = 25.0;
float stiffness = 0.00012;
float bounceHeight = 75.0;
float bounceSpeed = 1.16;
float ballSize = 21.0;

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;
attribute vec2 rm_TexCoord0;

varying vec2 vTexCoord;
varying vec3 vNormal;



void main(void)
{

   // Just flip and scale the model to fit us
   vec4 pos;
  
   pos.xyz = 0.2 * rm_Vertex.yzx - 1.0;
   pos.w   = 1.0;

   gl_Position = view_proj_matrix * pos;

   vTexCoord    = vec2(rm_Vertex.xy * 0.01 + 0.5);

   vNormal   =  mat3(view_matrix) * rm_Normal.yzx; 

}