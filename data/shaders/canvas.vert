#version 420 core

const vec2 vertices[6] = {
	{0.0, 0.0},     //    5----4 2
	{1.0, 0.0},     //    |   / /|
	{1.0, 1.0},     //    |  / / |							        
	{0.0, 0.0},     //    | / /  |
	{1.0, 1.0},     //    |/ /   |
	{0.0, 1.0},	    //    3 0----1
};

layout(std140, binding = 0) uniform Matrices {
	mat4 viewport;
	mat4 projection;
};

layout(std140, binding = 2) uniform TileData {
	ivec2 size;
	ivec2 position;
} tile_data;

out vec2 texcoord;

void main() {
	const vec2 position = (vertices[gl_VertexID] + tile_data.position) * tile_data.size;

	texcoord = vertices[gl_VertexID];
	gl_Position = projection * viewport * vec4(position, 0.0, 1.0);
}