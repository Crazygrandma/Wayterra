precision mediump float;

uniform int uRenderMode;

varying vec2 vUV;

uniform float windowWidth;
uniform float windowHeight;
uniform float numTiles;
uniform float uOffset;
uniform sampler2D tileMap;
uniform vec2 tileMapSize;
uniform sampler2D atlasTexture;
uniform sampler2D playerTexture;
uniform sampler2D logoTexture;



float getAspect() {
    return windowWidth / windowHeight;
}

// Compute the tile index and local UV inside a tile
void getTileCoordAndUV(vec2 uv, float numTiles, out vec2 tileCoord, out vec2 tileUVs) {
    vec2 scaledUV = uv * numTiles;
    tileCoord = floor(scaledUV);   // which tile
    tileUVs   = fract(scaledUV);   // local UV inside tile
}

vec3 getTileColor(vec3 tileData, vec2 tileCoord, float numTiles, vec2 tileUVs) {

    vec2 atlasGrid, tileSize;

    int tileIndex = int(tileData.r * 255.0);

    atlasGrid = vec2(8.0,6.0);
    tileSize = 1.0 / atlasGrid;
    //
    float cols = atlasGrid.x;
    //
    vec2 tileOffset;
    tileOffset.x = mod(float(tileIndex), cols) * tileSize.x;
    tileOffset.y = floor(float(tileIndex) / cols) * tileSize.y;

    tileUVs = tileUVs * tileSize + tileOffset;

    return texture2D(atlasTexture, tileUVs).rgb;
}

void main() {
    vec2 uv = vUV;

    vec4 tex;

    if (uRenderMode == 0) {
        uv.x *= getAspect();
        vec2 tileCoord, tileUVs;
        getTileCoordAndUV(uv, numTiles, tileCoord, tileUVs);

        vec2 mapUV = (tileCoord + 0.5) / tileMapSize;
        vec3 tileData = texture2D(tileMap, mapUV).rgb;

        vec3 colorRgb = getTileColor(tileData, tileCoord, numTiles, tileUVs);
        gl_FragColor = vec4(colorRgb, 1.0);

    } else if (uRenderMode == 1) {
        
        vec4 tex = texture2D(playerTexture, uv);
        //vec4 tex = vec4(1.0);
        gl_FragColor = tex;

    } else if (uRenderMode == 2) {
        // UI / Logo
        vec4 tex = texture2D(logoTexture, uv);
        //vec4 tex = vec4(1.0);
        gl_FragColor = tex;
    }
}
