#version 420 core

const vec2 vertices[3] = {
	{-1.0, -1.0},
	{3.0, -1.0},
	{-1.0, 3.0},
};

layout(std140, binding = 0) uniform Matrices {
	mat4 view;
	mat4 proj;
} viewport;

out vec2 v_texcoord;

void main() {
	v_texcoord = vertices[gl_VertexID] / 2.0 + 0.5;
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}