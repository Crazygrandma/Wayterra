#version 100

precision mediump float;

varying vec2 vUV;

uniform sampler2D playerTexture;

void main()
{
    vec2 uv = vUV;

    gl_FragColor = texture2D(playerTexture, uv);
}
