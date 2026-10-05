#version 100

precision mediump float;

varying vec2 vUV;

uniform sampler2D playerTexture;
uniform float uTime;

void main()
{
    // Player size
    vec2 size = vec2(0.4);

    // Movement speed
    vec2 speed = vec2(0.20, 0.18);

    // Position over time
    vec2 position = speed * uTime;

    // Bounce between the edges
    position = abs(fract(position) * 2.0 - 1.0);

    // Keep the sprite inside the screen
    position = position * (1.0 - size) + size * 0.5;

    // Convert screen UV into player texture UV
    vec2 playerUV = (vUV - position) / size + 0.5;

    if (
        playerUV.x >= 0.0 &&
        playerUV.x <= 1.0 &&
        playerUV.y >= 0.0 &&
        playerUV.y <= 1.0
    ) {
        gl_FragColor = texture2D(playerTexture, playerUV);
    } else {
        gl_FragColor = vec4(0.05, 0.08, 0.12, 1.0);
    }
}
