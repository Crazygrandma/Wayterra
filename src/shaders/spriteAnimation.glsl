#version 100

precision mediump float;

varying vec2 vUV;

uniform float uTime;
uniform float uRow;           // animation row in the sheet (0 = top row)
uniform float uRowCount;      // total rows in the sheet
uniform float uFrameCount;    // frames in the current animation
uniform float uTimePerFrame;  // seconds per frame
uniform float uCellWidthUV;   // cell width in UV space (cellWidth / sheetWidth)
uniform bool uInverted;
uniform sampler2D animationTexture;

void main()
{
    float frame = mod(floor(uTime / uTimePerFrame), uFrameCount);

    vec2 uv;
    // uCellWidthUV (not uFrameCount) defines the horizontal slice, so an
    // animation shorter than the sheet width still samples a single cell
    float fx = uInverted ? 1.0 - vUV.x : vUV.x; // mirror within the cell
    uv.x = (frame + fx) * uCellWidthUV;
    // 1.0 - vUV.y flip cancels the Y flip of the FBO -> scene buffer path,
    // so rows stay indexed top-down (uRow 0 = top row of the sheet)
    uv.y = (uRow + 1.0 - vUV.y) / uRowCount;

    gl_FragColor = texture2D(animationTexture, uv);
}
