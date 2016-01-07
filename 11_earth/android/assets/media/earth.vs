#version 150
uniform mat4 world_view_proj_matrix;
uniform mat4 world_view_matrix;
// Earth vertex shader computes lighting coefficients.
//
// Note: For optimal light animation set the RenderMonkey cycle time 
// for predefined variables to 20 seconds.  (Found in the Edit->Preferences).

// predefined variables used to animate light direction
uniform float cos_time_0_2PI, sin_time_0_2PI;

// artist variable to control season
float season = 0.0;

// attributes needed for OpenGL ES
attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;
attribute vec2 rm_TexCoord0;

// varying variables passed to fragment shader
varying float Diffuse;
varying vec3  Specular;
varying vec2  TexCoord;

void main(void)
{
    // calculate vertex position in eye coordinates
    vec4 ecPosition = world_view_matrix * vec4(rm_Vertex,1.0);
    
    // compute the transformed normal
    vec3 tnorm      = normalize(mat3(world_view_matrix)*rm_Normal);

    // compute the light vector pointing toward the sun, in model coordinates
    // x,y compose the longitude and z the (seasonal) lattitude of the nadir point.
    vec3 lightVec = normalize(mat3(world_view_matrix) * vec3(cos_time_0_2PI, season,sin_time_0_2PI));

    // compute the reflection vector
    vec3 reflectVec = reflect(-lightVec, tnorm);
    
    // compute a unit vector in direction of viewing position
    vec3 viewVec    = normalize(vec3 (-ecPosition));

    // Calculate specular light intensity, scale down and
    // apply a slightly yellowish tint. 
    float specIntensity = pow(max(dot(reflectVec, viewVec), 0.0), 8.0);
    specIntensity       = 0.3 * specIntensity; 
    Specular            = specIntensity * vec3 (1.0, 0.941, 0.898);
    
    // Calculate a diffuse light intensity
    Diffuse             = dot(lightVec, tnorm);

    // Pass texture coordinates fragment shader
    TexCoord    = vec2 (rm_TexCoord0);
    
    // output final vertex information
    gl_Position     = world_view_proj_matrix * vec4(rm_Vertex,1.0);
}
