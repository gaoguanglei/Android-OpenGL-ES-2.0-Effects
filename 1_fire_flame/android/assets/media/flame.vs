uniform float time_0_X;

vec4 layer_speed = vec4(0.68, 0.53, 0.75, 1.0);

attribute vec3 rm_Vertex;
attribute vec2 rm_TexCoord0;

varying vec2  vTexCoord0;
varying vec2  vTexCoord1;
varying vec2  vTexCoord2;
varying vec2  vTexCoord3;

void main(void)
{
   // Align quad with the screen
   gl_Position = vec4( rm_Vertex.x, rm_Vertex.y, 0.0, 1.0);
   
   // Output TexCoord0 directly
   // OpenGL Y's are upsidedown from D3D's... lets invert the y value here
   // instead of per pixel.
   vTexCoord0 = vec2(rm_TexCoord0.x, rm_TexCoord0.y);

   // Base texture coordinates plus scaled time
   vTexCoord1.x = rm_TexCoord0.x;
   vTexCoord1.y = (rm_TexCoord0.y) - layer_speed.x * time_0_X;

   // Base texture coordinates plus scaled time
   vTexCoord2.x = rm_TexCoord0.x;
   vTexCoord2.y = (rm_TexCoord0.y) - layer_speed.y * time_0_X;

   // Base texture coordinates plus scaled time
   vTexCoord3.x = rm_TexCoord0.x;
   vTexCoord3.y =(rm_TexCoord0.y) - layer_speed.z * time_0_X;

}