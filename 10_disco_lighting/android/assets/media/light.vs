uniform mat4 view_proj_matrix;
uniform mat4 light_matrix;

vec4 lightPos = vec4(0,0,30.0,1);

attribute vec3 rm_Vertex;

varying vec3  vDir;

void main(void)
{
   vec4 Pos = vec4(rm_Vertex,1.0);
   
   vec3 dir = normalize(Pos.xyz);
   Pos.xyz = 10.0 * dir + lightPos.xyz;   

   gl_Position = view_proj_matrix * Pos;


   // Create a rotation matrix from three Euler angles.
   // This work is better done on the CPU as it is constant for the
   // whole scene. In RenderMonkey we'll have to do it in the shader though.
   
   // Rotate our fancy shading
   vDir = mat3(light_matrix) * dir;

}
