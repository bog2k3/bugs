#version 120

// Input vertex data, different for all executions of this shader.
attribute vec2 vertexPosition_screenspace;
attribute vec2 vertexUV;

uniform vec2 viewportHalfSize;

// Output data ; will be interpolated for each fragment.
varying vec2 UV;

void main(){

	// Output position of the vertex, in clip space
	// map [0..vW][0..vH] to [-1..1][-1..1]
	vec2 vertexPosition_homoneneousspace = vertexPosition_screenspace - viewportHalfSize + 0.5f;
	vertexPosition_homoneneousspace /= viewportHalfSize;
	gl_Position =  vec4(vertexPosition_homoneneousspace,0,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
}

