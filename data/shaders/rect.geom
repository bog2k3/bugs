#include "util.vert"

layout(points) in;
layout(line_strip, max_vertices = 5) out;

in vec2 vSize[];
in float vRotation[];
in vec3 vColor[];

out vec3 fColor;

void main() {
	mat4 mRot = RotationMatrix(vRotation[0]);
	vec4 vCenter = gl_in[0].gl_Position;
	vec2 halfSize = vSize[0] * 0.5;

	fColor = vColor[0];
	
	vec4 initial = mRot * (vCenter + vec2(-halfSize.x, -halfSize.y));
	gl_Position = initial;
	EmitVertex();
	gl_Position = mRot * (vCenter + vec2(+halfSize.x, -halfSize.y));
	EmitVertex();
	gl_Position = mRot * (vCenter + vec2(+halfSize.x, +halfSize.y));
	EmitVertex();
	gl_Position = mRot * (vCenter + vec2(-halfSize.x, +halfSize.y));
	EmitVertex();
	gl_Position = initial;
	EmitVertex();
	
	EndPrimitive();
}