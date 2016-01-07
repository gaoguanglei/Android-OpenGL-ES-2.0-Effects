#version 150
uniform mat4 view_matrix;
uniform mat4 view_proj_matrix;
uniform float localTime;

float bounceHeight = 75.0;
float bounceSpeed = 1.16;
float ballSize = 21.0;

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;

varying vec3 normal;
varying vec3 viewVec;

void main(void)
{
   // Normalize and scale, just because the source
   // model is not a perfect sphere around origin
   vec3 Pos = ballSize * normalize(rm_Vertex);
   
   // Create a bouncy movement
   float t = fract( localTime * bounceSpeed )  ;
   t *= 4.0 * (1.0 - t) ;
 
   Pos.z += bounceHeight * t;

   gl_Position = view_proj_matrix * vec4(Pos,1.0);

   normal = mat3(view_matrix) * rm_Normal;      
   
   viewVec = vec3(view_matrix * vec4(Pos,1.0));
}