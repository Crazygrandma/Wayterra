#version 100

precision mediump float;

varying vec2 vUV;

uniform float uTime;
uniform bool uInverted;
uniform sampler2D animationTexture;

void main()
{
    float frame = mod(floor(uTime * 10.0), 19.0);

    vec2 uv;
    uv.x = (vUV.x + frame) / 19.0;
    uv.y = 1.0 - vUV.y;

    gl_FragColor = texture2D(animationTexture, uv);
}
