export class ChunkLayer {
    constructor(chunks = []) {
        this.chunks = chunks;
        this.dirty = true; // initially needs rendering
    }

    addChunk(chunk) {
        this.chunks.push(chunk);
        this.dirty = true; // mark layer dirty when new chunk added
    }

    render(ctx, viewport = null) {
        for (const chunk of this.chunks) {
            for (let y = 0; y < chunk.height; y++) {
                for (let x = 0; x < chunk.width; x++) {
                    const tileType = chunk.tiles[y][x];
                    if (tileType === 'empty') continue;

                    const worldPos = chunk.getTileWorldPosition(x, y);

                    ctx.fillStyle = tileType === 'platform' ? 'saddlebrown' : 'grey';
                    ctx.fillRect(
                        Math.round(worldPos.x),
                        Math.round(worldPos.y),
                        chunk.tileSize,
                        chunk.tileSize
                    );
                }
            }
        }
        this.dirty = false; // clean after render
    }

    getDamage() {
        if (!this.dirty) return null;

        // Simple: mark the entire layer as damaged
        return {
            x: 0,
            y: 0,
            width: Infinity, // or canvas.width if you prefer
            height: Infinity // or canvas.height
        };
    }
}
