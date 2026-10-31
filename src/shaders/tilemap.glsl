#version 100
precision mediump float;

varying vec2 vUV;

uniform sampler2D tilesetTexture;
uniform float tileId;

void main()
{
    float tilesetSize = 8.0;
    float tileSize = 1.0 / tilesetSize;

    float column = mod(tileId, tilesetSize);
    float row = floor(tileId / tilesetSize);

    vec2 uv = vec2(vUV.x, 1.0 - vUV.y);
    uv *= tileSize;

    uv.x += column * tileSize;
    uv.y += row * tileSize;

    gl_FragColor = texture2D(tilesetTexture, uv);
}
