#version 100
precision mediump float;

varying vec2 vUV;

uniform sampler2D tilemapTexture;
uniform float tileId;

void main()
{
    vec2 uv = vUV;

    // Each tile occupies half the texture width.
    uv.x = (uv.x + tileId) / 2.0;
    gl_FragColor = texture2D(tilemapTexture, uv);
}
