#version 100

precision mediump float;

varying vec2 vUV;

uniform sampler2D spriteTexture;

void main()
{
    vec2 uv = vec2(vUV.x, 1.0 - vUV.y);
    gl_FragColor = texture2D(spriteTexture, uv);
    



    // DEBUG
    //gl_FragColor = vec4(uv.x, uv.y, 0.0f, 1.0f);
}
