attribute vec2 aPos;
attribute vec2 aUV; // must be used
varying vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
