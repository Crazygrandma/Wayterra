precision mediump float;

varying vec2 vUV;

uniform float windowWidth;
uniform float windowHeight;
uniform float numTiles;
uniform float uOffset;
uniform sampler2D tileMap;
uniform vec2 tileMapSize;
// TODO add toggle to switch from tile renderer to sprite renderer

uniform sampler2D atlasTexture;


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
    // Flip Y so tilemap matches screen
    vec2 uv = vUV;
    uv.x += uOffset;

    // Keep tiles square
    uv.x *= getAspect();


    vec2 tileCoord, tileUVs;
    getTileCoordAndUV(uv, numTiles, tileCoord, tileUVs);

    // Sample tilemap
    vec2 mapUV = (tileCoord + 0.5) / tileMapSize;
    vec3 tileData = texture2D(tileMap, mapUV).rgb;

    // Determine final color
    // vec3 color = vec3(tileUVs, 0.0);
    vec3 color = getTileColor(tileData, tileCoord, numTiles, tileUVs);
    // vec3 color = texture2D(atlasTexture,uv).rgb;
    gl_FragColor = vec4(color, 1.0);
}
