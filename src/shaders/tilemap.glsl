#version 100
precision mediump float;

varying vec2 vUV;

uniform sampler2D tilemapTexture;
uniform float tileId;

void main()
{
    float tileHeight = 0.5;

    vec2 uv = vec2(
        vUV.x,
        (1.0 - vUV.y) * tileHeight + tileId * tileHeight
    );

    gl_FragColor = texture2D(tilemapTexture, uv);
}
