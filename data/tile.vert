#version 420 core

const vec2 vertices[6] = {
	{0.0, 0.0},
	{1.0, 0.0},
	{1.0, 1.0},							        
	{0.0, 0.0},
	{1.0, 1.0},
	{0.0, 1.0},
};

layout(std140, binding = 0) uniform Matrices {
	mat4 view;
	mat4 proj;
} viewport;

layout(std140, binding = 1) uniform TileData {
	ivec2 coord;
	uint layer;
	uint size;
} tile_data;

out vec2 v_texcoord;

void main() {
	v_texcoord = vertices[gl_VertexID];

	const vec2 position = (vertices[gl_VertexID] + vec2(tile_data.coord)) * tile_data.size;
	gl_Position = viewport.proj * viewport.view * vec4(position, 0.0, 1.0);
}