attribute vec2 aPos;
attribute vec2 aUV; // must be used
varying vec2 vUV;
uniform vec2 uOffset;

void main() {
    vUV = aUV;
    gl_Position = vec4(aPos + uOffset, 0.0, 1.0);
}
