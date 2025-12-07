export class Compositor {
    constructor(ctx) {
        this.ctx = ctx;
        this.layers = [];
    }

    addLayer(layer) {
        this.layers.push(layer);
    }

    render() {
        // Collect damage regions
        const damageRegions = [];
        for (const layer of this.layers) {
            const damage = layer.getDamage?.();
            if (damage) damageRegions.push(damage);
        }

        if (damageRegions.length === 0) return; // nothing to redraw

        for (const region of damageRegions) {
            // clear damaged region
            this.ctx.clearRect(
                Math.floor(region.x),
                Math.floor(region.y),
                Math.ceil(region.width),
                Math.ceil(region.height)
            );


            // redraw layers intersecting this region
            for (const layer of this.layers) {
                const layerBox = {
                    x: layer.x ?? layer.prevX ?? 0,
                    y: layer.y ?? layer.prevY ?? 0,
                    width: layer.width ?? layer.size ?? 0,
                    height: layer.height ?? layer.size ?? 0
                };

                if (Compositor.rectIntersect(layerBox, region)) {
                    layer.render(this.ctx);
                    
                }
            }
        }
    }

    static rectIntersect(a, b) {
        return !(a.x + a.width < b.x ||
                 a.x > b.x + b.width ||
                 a.y + a.height < b.y ||
                 a.y > b.y + b.height);
    }
}
