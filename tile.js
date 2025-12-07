export class Tile {
    constructor(x, y, type, size = 50) {
        this.x = x;
        this.y = y;
        this.type = type;
        this.size = size;
        this.dirty = true; // initially needs rendering
    }

    render(ctx) {
        ctx.fillStyle = this.type === 'platform' ? 'saddlebrown' : 'grey';
        ctx.fillRect(this.x, this.y, this.size, this.size);
        this.dirty = false; // mark as clean after render
    }

    getDamage() {
        if (!this.dirty) return null;
        return { x: this.x, y: this.y, width: this.size, height: this.size };
    }
}
