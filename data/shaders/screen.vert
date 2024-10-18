#version 420 core

const vec2 vertices[3] = {
	{-1.0, -1.0},
	{3.0, -1.0},
	{-1.0, 3.0},
};

out vec2 texcoord;

void main() {
	texcoord = vertices[gl_VertexID] / 2.0 + 0.5;
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}