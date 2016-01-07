#version 150
uniform mat4 view_proj_matrix;
uniform mat4 inv_view_matrix;

uniform mat4  mWorld;
uniform vec4  ambient;
uniform mat4  texture_space_matrix;
uniform float fAmbient;
uniform vec4  vRingCenter;
uniform vec4  vRingScale;
uniform vec4  light2Position;
uniform vec4  light1Position;

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;

varying vec4 vNormal;      // Has ambient in W
varying vec3 vView;
varying vec3 vDirAniso;
varying vec4 vTexPos;   // Has light intensity in W
varying vec3 vLight1;
varying vec3 vLight2;

void main(void)
{
   
   vec3 pos = vec3(-rm_Vertex.x, -rm_Vertex.z, rm_Vertex.y);
   vec3 normal = vec3(-rm_Normal.x, -rm_Normal.z, rm_Normal.y);
   // Texture space coordinates:
   vec3 texPos      = pos - vRingCenter.xyz;
   vec3 texSpacePos = texPos * vRingScale.xyz + vec3(0.0, -40.0, -40.0);
   
   // keep in mind that OpenGL is column major, while Direct3D is row major.
   // so this sets up a column matrix.
 //  mat3 texSpaceMat = mat3(vec3(texture_space_matrix[0]),vec3(texture_space_matrix[1]),vec3(texture_space_matrix[2]));


   // OpenGL treats this as row vector * column major matrix
 //  texSpacePos = texSpacePos * texSpaceMat;
 
 texSpacePos = texSpacePos * mat3(texture_space_matrix);

   vTexPos.xyz = texSpacePos;

   // Ambient lighting: 
   vTexPos.w = ambient.x * 5.0;

   // Compute direction of anisotropy
   vec3 dirAniso = cross(normal, normalize(texSpacePos.xyz));

   gl_Position = view_proj_matrix * vec4(pos,1.0);
 
   // Output the normal vector ( w component contains ambient factor ) 
   vNormal   = vec4( normal.xyz * 0.5 + vec3(0.5,0.5,0.5), fAmbient );

   // Propagate direction of anisotropy:
   vDirAniso = dirAniso * 0.5 + vec3(0.5,0.5,0.5);

   // Compute camera position:
   vec4 vCameraPosition;
   // OpenGL is a different handedness than d3d...so we 
   // have to reverse the arguments for this transformation.
   vCameraPosition.xyzw = vec4(0.0, 0.0, 0.0, 1.0) * inv_view_matrix;

   // Calculate view vector:
   vView = normalize( vCameraPosition.xyz - vec3(mWorld * vec4(pos,1.0) ));

   // Compute light direction vectors: 
   // OpenGL is a different handedness than d3d...so we 
   // have to reverse the arguments for these transformations.
   vec3 tempLP1 = (light1Position.xyzw * inv_view_matrix).xyz;
   vec3 tempLP2 = (light2Position.xyzw * inv_view_matrix).xyz;
   
   vLight1   = normalize(tempLP1.xyz - pos );
   
   vLight2   = normalize(tempLP2.xyz - pos );
   
}