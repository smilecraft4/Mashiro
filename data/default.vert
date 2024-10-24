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

out vec2 v_texcoord;

void main() {
	const vec2 position = vertices[gl_VertexID] * 256;

	v_texcoord = vertices[gl_VertexID];
	gl_Position = viewport.proj * viewport.view * vec4(position, 0.0, 1.0);
}