attribute vec3 vPos;
attribute vec3 vNormal;
attribute vec4 vColor;
attribute vec2 vUV1;
 
varying vec4 fColor;
varying vec2 fUV1;
varying vec3 fNormal;

uniform mat4 mPVW;
 
void main() {
    gl_Position = mPVW * vec4(vPos, 1);
    fNormal = (mPVW * vec4(vNormal, 0)).xyz;
    fColor = vColor;
    fUV1 = vUV1;
}
