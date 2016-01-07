vec3 LightDir = vec3(0.0,0.0,1.0);
uniform mat4 world_view_proj_mat;

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;
attribute vec3 rm_TexCoord0;
attribute vec3 rm_Tangent;
attribute vec3 rm_Binormal;

varying vec2 texCoord;
varying vec3 normal;
varying vec3 lightDirInTangent;

void main(void)
{
   gl_Position = world_view_proj_mat * vec4(rm_Vertex,1.0);
   
   texCoord = rm_TexCoord0.xy;
   
   mat3 tangentMat = mat3(rm_Tangent,
                          rm_Binormal,
                          rm_Normal);
   lightDirInTangent = normalize(LightDir) * tangentMat;

   normal = rm_Normal;
}