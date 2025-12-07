export class Chunk {
    constructor(chunkX, chunkY, tileSize = 50, width = 16, height = 16) {
        this.chunkX = chunkX;
        this.chunkY = chunkY;
        this.tileSize = tileSize;
        this.width = width;
        this.height = height;

        // 2D array of tile types ('empty' or 'platform')
        this.tiles = Array.from({ length: height }, () => Array(width).fill('empty'));
    }

    setTile(x, y, type) {
        if (x < 0 || x >= this.width || y < 0 || y >= this.height) return;
        this.tiles[y][x] = type;
    }

    getTileWorldPosition(x, y) {
        return {
            x: this.chunkX * this.width * this.tileSize + x * this.tileSize,
            y: this.chunkY * this.height * this.tileSize + y * this.tileSize
        };
    }
}
