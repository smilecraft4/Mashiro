- TODO:
Create a tile class
Get the AABB for the viewport with rotation, scale, and position
Get the AABB for a tile with the viewport rotation, scale and position
Cull every tiles not in the viewport view

Rename canvas.glsl to tile.glsl
Implement caching for tiles

Benchmark the speed of saving different size of tile texture start with opengl->glTexImage()->std::vector->saveImage (with different settings of compression)
Benchmark the speed of loading different size of tile texture memory->glGenTexture->render

Test the speed of saving/loading from file
Test using multithreading I/O to load a lot of tiles at the same time

think of a better tile generation algorithm (lazy-loading)
test the efficency of textureArray

// Benchmark the speed between uniform or uniform buffer for tiles


// Test AABB
// Test Culling
// Test loading
// Test Unloading

// Benchmark Render
// Benchmark Tile Saving
// Benchmark Tile Openning
// Bennchmark File Saving
// Bennchmark File Openning