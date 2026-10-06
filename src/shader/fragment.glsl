#version 100

precision mediump float;

varying vec2 vUV;

uniform float uTime;
uniform bool uAnimate;
uniform sampler2D spriteTexture;

void main()
{
    vec2 uv = vUV;

    if (uAnimate) {
        uv.y += sin(uTime * 2.0) * 0.05;
    }

    gl_FragColor = texture2D(spriteTexture, uv);
}
