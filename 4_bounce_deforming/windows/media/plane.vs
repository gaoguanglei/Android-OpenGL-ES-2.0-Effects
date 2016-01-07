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
varying vec3 vViewVec;


// Offsets at which we sample the heights to form the normal
 vec2 offX = vec2(1.0, 0.0);
 vec2 offY = vec2(0.0, 1.0);

float sinc(float x)
{
   return sin(x) / x;
}

// Returns the height at the given position
float getZ(vec2 Pos, float push, float ballz)
{
   
   float punch = exp2(-stiffness * dot(Pos,Pos));

   ballz -= sqrt(max(ballSize * ballSize - dot(Pos, Pos), 0.0));

   // Make sure the ball never leaks through
   return min(punch * push, ballz);
}

void main(void)
{
  // Our model is a little too large ...
   vec4 Pos = vec4(rm_Vertex,1.0);

   Pos.xyz *=  0.4;
  // The balls bouncy movement
  float s = fract(bounceSpeed * localTime);
  float t = 4.0 * s * (1.0 - s);

   // The height at which the ball currently is, slightly
   // offset to ensure no leaks.
   float ballz = bounceHeight * t - 3.0;

   // The vibrating movement of the surface
   float push = -ballSize * sinc(hardness * s);
   // Sample at current position and two neighbors
   
   float z0 = getZ(Pos.xy, push, ballz);
   
   float z1 = getZ(Pos.xy + offX, push, ballz);
   
   float z2 = getZ(Pos.xy + offY, push, ballz);
   
   Pos.z = z0;

   gl_Position = view_proj_matrix * Pos;

     
   vec3 tangent  = vec3(offX, z1 - z0);
   vec3 binormal = vec3(offY, z2 - z0);

   // Construct the normal
   vNormal = normalize(cross(tangent, binormal));


     
   // Eye-space lighting
   vNormal     = mat3(view_matrix) * vNormal; 
   
   vViewVec    = vec3(view_matrix * Pos);
   
   vTexCoord   = vec2(rm_TexCoord0);
   
}