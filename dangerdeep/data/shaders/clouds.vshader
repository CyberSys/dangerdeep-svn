// -*- mode: C; -*-

varying vec2 texcoord;
varying vec3 lightdir;
varying vec3 lightcolor;

void main()
{
	texcoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vec3(gl_Vertex) * gl_LightSource[0].position.w);
	lightdir = lightdir_obj;

	// finally compute position
	gl_Position = ftransform();

	lightcolor = gl_Color.xyz;

	// set fog coordinate
//	gl_FogFragCoord = gl_Position.z;
}