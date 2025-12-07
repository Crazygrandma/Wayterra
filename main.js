import { Player } from './player.js';
import { Tile,Chunk } from './tile.js';
import { ChunkLayer } from './chunkLayer.js';
import { Compositor } from './compositor.js';

const canvas = document.getElementById('gameCanvas');
const ctx = canvas.getContext('2d');

canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

const compositor = new Compositor(ctx);

// Create tiles and player
const tiles = [
    new Tile(100, 400, 'platform'),
    new Tile(150, 400, 'platform'),
    new Tile(200, 400, 'platform')
];

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
    player.update(input, tiles);   // update player
    compositor.render();           // render only damaged regions
    requestAnimationFrame(gameLoop);
}

gameLoop();




// // Create a chunk and fill bottom row
// const chunk = new Chunk(0, 0);
// for (let x = 0; x < chunk.width; x++) {
//     chunk.setTile(x, chunk.height - 1, 'platform');
// }

// // Create a chunk layer and add chunk
// const chunkLayer = new ChunkLayer();
// chunkLayer.addChunk(chunk);

// // Add to compositor
// compositor.addLayer(chunkLayer);

// // Game loop
// function gameLoop() {
//     compositor.render();
//     requestAnimationFrame(gameLoop);
// }

// gameLoop();
