import { Player } from './player.js';
import { Tile } from './tile.js';
import { Compositor } from './compositor.js';

const canvas = document.getElementById('gameCanvas');
const ctx = canvas.getContext('2d');

canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

const compositor = new Compositor(ctx);

// Create tiles and player

const tiles = [];
const startX = 100;
const y = 400;
const count = 46; // number of tiles
const spacing = 50;

for (let i = 0; i < count; i++) {
    tiles.push(new Tile(startX + i * spacing, y, 'platform'));
}


const player = new Player(120, 300);

// Add layers to compositor
tiles.forEach(tile => compositor.addLayer(tile));
compositor.addLayer(player);

// Track keyboard state
const input = {};
window.addEventListener('keydown', e => input[e.key] = true);
window.addEventListener('keyup', e => input[e.key] = false);

// Game loop
function gameLoop() {
    // Update player with current input
    player.update(input, tiles);

    // Render all layers
    compositor.render();

    requestAnimationFrame(gameLoop);
}

gameLoop();
