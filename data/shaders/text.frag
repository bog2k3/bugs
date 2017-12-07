#version 120

// Interpolated values from the vertex shaders
varying vec2 UV;
varying vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){

	gl_FragColor = color * texture2D( myTextureSampler, UV );
	
	
}
