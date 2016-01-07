#version 150

uniform mat4 view_proj_matrix;
uniform mat4 light_matrix;
uniform mat4 inv_view_matrix;


vec4 lightPos = vec4(0,0,30.0,1);

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;


varying vec2  vTexCoord;
varying vec3  vLightVec;
varying vec3  vViewVec;
varying vec3  vDir;

void main(void)
{
   // Scale and flip
   vec4 Pos = vec4(rm_Vertex,1.0);
   vec4 n_pos = normalize(Pos);

   vec3 normal = -rm_Normal;
   
     
  // vTexCoord = vec2(gl_MultiTexCoord0);

   // The model we're using doesn't contain any texture coordinates,
   // however,being a simple cube we can easily generate them ourselves.
   vTexCoord.x = 0.5 * mix(n_pos.x, n_pos.y, abs(normal.x));
   vTexCoord.y = 0.5 * mix(n_pos.z, n_pos.y, abs(normal.z));

   // No texture coordinates means no tangent space, so we'll
   // have to generate it ourselves too.
   vec3 tangent  = -vec3(abs(normal.y) + abs(normal.z), abs(normal.x), 0);
   vec3 binormal = -vec3(0, abs(normal.z), abs(normal.x) + abs(normal.y));


 //  Pos.xyz *= -200.0;
   gl_Position = view_proj_matrix * Pos;

   // Transform light vector into tangent space
   vec3 lightVec = lightPos.xyz - Pos.xyz;
   vLightVec.x = dot(lightVec, tangent);
   vLightVec.y = dot(lightVec, binormal);
   vLightVec.z = dot(lightVec, normal);

   vec4 view_position = inv_view_matrix * vec4(0.0,0.0,0.0,1);

   // Transform view vector into tangent space
   vec3 viewVec = view_position.xyz - Pos.xyz;
   vViewVec.x = dot(viewVec, tangent);
   vViewVec.y = dot(viewVec, binormal);
   vViewVec.z = dot(viewVec, normal);

   // Direction to look up our fancy shading at
   vec3 dir = -lightVec;

   // Create a rotation matrix from three Euler angles.
   // This work is better done on the CPU as it is constant for the
   // whole scene. In RenderMonkey we'll have to do it in the shader though.

   // Rotate our fancy shading
   vDir = mat3(light_matrix) * dir;


}
