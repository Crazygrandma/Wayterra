#version 100

precision mediump float;

varying vec2 vUV;

uniform float uTime;

void main()
{
    vec2 uv = vUV * 12.0;

    vec2 tile = floor(uv);
    vec2 local = fract(uv);

    float wave =
        sin(tile.x * 1.7 + uTime)
        * sin(tile.y * 1.3 + uTime * 0.7);

    float edge =
        step(0.05, local.x)
        * step(0.05, local.y)
        * step(local.x, 0.95)
        * step(local.y, 0.95);

    float value = wave * 0.5 + 0.5;

    vec3 color = mix(
        vec3(0.05, 0.08, 0.12),
        vec3(0.15, 0.5, 0.8),
        value
    );

    color *= edge;

    gl_FragColor = vec4(color, 1.0);
}
