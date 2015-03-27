attribute vec3 vPos;
attribute vec4 vColor;
 
varying vec4 fColor;

uniform mat4 mViewProj;
 
void main() {
 
    gl_Position = mViewProj * vec4(vPos, 1);
    fColor = vColor;
}
