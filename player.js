export class Player {
    constructor(x, y, width = 40, height = 60) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.velY = 0;
        this.speed = 2;
        this.gravity = 1;
        this.onGround = false;
        this.prevX = x;
        this.prevY = y;
        this.dirty = true;
    }

    update(input, tiles) {
        this.prevX = this.x;
        this.prevY = this.y;

        if (input['ArrowLeft']) this.x -= this.speed;
        if (input['ArrowRight']) this.x += this.speed;
        if ((input[' '] || input['ArrowUp']) && this.onGround) this.velY = -10;

        this.velY += this.gravity;
        this.y += this.velY;

        // Collision detection
        this.onGround = false;
        for (const tile of tiles) {
            if (this.x < tile.x + tile.size &&
                this.x + this.width > tile.x &&
                this.y < tile.y + tile.size &&
                this.y + this.height > tile.y) {

                if (this.velY > 0) {
                    this.y = tile.y - this.height;
                    this.velY = 0;
                    this.onGround = true;
                }
            }
        }

        // Mark as dirty if moved
        this.dirty = this.x !== this.prevX || this.y !== this.prevY;
    }

    render(ctx) {
        ctx.fillStyle = 'blue';
        ctx.fillRect(this.x, this.y, this.width, this.height);
        this.dirty = false; // clean after render
    }

    getDamage() {
        if (!this.dirty) return null;
        return {
            x: Math.min(this.prevX, this.x),
            y: Math.min(this.prevY, this.y),
            width: this.width + Math.abs(this.x - this.prevX),
            height: this.height + Math.abs(this.y - this.prevY)
        };
    }
}
