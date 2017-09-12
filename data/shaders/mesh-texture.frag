varying vec4 fColor;
varying vec2 fUV1;
varying vec3 fNormal;

uniform sampler2D tex1;

void main() {
	gl_FragColor = fColor;// * texture(tex1, fUV1);
}
