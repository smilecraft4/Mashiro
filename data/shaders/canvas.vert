#version 420 core

struct Vertex {
	vec2 position;
	vec2 coord;
};

Vertex vertices[6] = {
	{{-0.5, -0.5}, {0.0, 0.0}},     //    5----4 2
	{{ 0.5, -0.5}, {1.0, 0.0}},     //    |   / /|
	{{ 0.5,  0.5}, {1.0, 1.0}},     //    |  / / |							        
	{{-0.5, -0.5}, {0.0, 0.0}},     //    | / /  |
	{{ 0.5,  0.5}, {1.0, 1.0}},     //    |/ /   |
	{{-0.5,  0.5}, {0.0, 1.0}},	    //    3 0----1
};

layout(std140, binding = 0) uniform Matrices {
	mat4 viewport;
	mat4 projection;
};

layout(std140, binding = 2) uniform TileData {
	ivec2 size;
	ivec2 position;
	mat4 model;
} tile_data;

// uniform mat4 model;

out vec2 texcoord;

void main() {
	gl_Position = projection * viewport * tile_data.model * vec4(vertices[gl_VertexID].position, 0.0, 1.0);
	texcoord = vertices[gl_VertexID].coord;
}