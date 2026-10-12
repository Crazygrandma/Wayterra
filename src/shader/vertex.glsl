attribute vec2 aPos;
attribute vec2 aUV;

uniform mat4 uProjection;
uniform mat4 uModel;

varying vec2 vUV;

void main()
{
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
