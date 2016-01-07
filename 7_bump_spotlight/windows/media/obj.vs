//vec3 LightDir = vec3(0.0,0.0,1.0);

uniform mat4 world_view_proj_mat;

uniform vec4 view_position;
uniform vec4 lightPos;
uniform vec4 lightDir;

attribute vec4 rm_Vertex;
attribute vec3 rm_Normal;
attribute vec2 rm_TexCoord0;
attribute vec3 rm_Tangent;
attribute vec3 rm_Binormal;

varying vec2 vTexCoord;
varying vec3 vLightVec;
varying vec3 vLightDir;
varying vec3 vViewVec;

//varying vec3 vTangent;
//varying vec3 vBinormal;
//varying vec3 vNormal;

void main(void)
{
    gl_Position = world_view_proj_mat * rm_Vertex;

   // construct a tangent space matrix from tan/binorm/norm   
   mat3 tanSpace = mat3( rm_Tangent, rm_Binormal,rm_Normal);
   
    // find lightvec and xform it to tanspace.
   vec3 tempVec = lightPos.xyz - rm_Vertex.xyz;
   vLightVec = tempVec * tanSpace;
   
   // xform lightdir to tanspace.
   tempVec = vec3(lightDir.x, lightDir.y, lightDir.z);
   vLightDir = tempVec * tanSpace;

   // find viewvec and xform it to tanspace.
   tempVec  = view_position.xyz - rm_Vertex.xyz;
   vViewVec = tempVec * tanSpace;
   
   // load texcoords.
   vTexCoord = rm_TexCoord0;
   
//   vTangent   = rm_Tangent;
//   vBinormal  = rm_Binormal;
//  vNormal    = rm_Normal;  
}