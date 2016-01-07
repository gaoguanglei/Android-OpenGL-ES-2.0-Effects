#version 150
uniform mat4 view_matrix;
uniform mat4 view_proj_matrix;
uniform float localTime;

float bounceHeight = 75.0;
float bounceSpeed = 1.16;
float ballSize = 21.0;
float bounceMin = -0.1;
float GroundHeight = 1.0;
float squeezeHeight = 1.4;

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;

varying vec3 vNormal;
varying vec3 vViewVec;

void main(void)
{
   // Normalize and scale, just because the source
   // model is not a perfect sphere around origin
   vec3 pos = 5.0 * normalize(vec3(rm_Vertex));

   // Basic Bounce
   float t = fract( localTime * bounceSpeed );
   float center = bounceHeight * t * (1.0 - t);
 
   pos.y += center + bounceMin;

   // Squeeze
   if (pos.y < GroundHeight)
   {
      // Squeeze in Z direction
      float squeeze = (1.0 - exp2(1.0 * (pos.y - GroundHeight)));
      pos.y = GroundHeight - squeeze * squeezeHeight;
      
      // Flatten in XZ direcion
      vec2 xyNorm = vec2(normalize(vec3(rm_Normal.xy,1.0)));
      pos.xz += squeeze * xyNorm * squeezeHeight;
         
   }

   gl_Position = view_proj_matrix * vec4(pos, 1.0);


   // gl_NormalMatrix the inverse of the upper 3x3 of the view matrix.
   vNormal   =  mat3(view_matrix)  * rm_Normal; 

   vViewVec   = vec3((view_matrix * vec4(pos, 1.0)));
}