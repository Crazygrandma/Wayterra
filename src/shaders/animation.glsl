#version 100

precision mediump float;

varying vec2 vUV;

uniform float uTime;
uniform bool uAnimate;
uniform sampler2D animatedSpriteTexture;

void main()
{

    vec2 uv = vec2(vUV.x, 1.0 - vUV.y);
    gl_FragColor = texture2D(spriteTexture, uv);
}
