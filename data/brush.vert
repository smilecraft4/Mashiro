#version 420 core

const vec2 vertices[6] = {
	{-0.5, -0.5},
	{ 0.5, -0.5},
	{ 0.5,  0.5},							        
	{-0.5, -0.5},
	{ 0.5,  0.5},
	{-0.5,  0.5},
};

layout(std140, binding = 0) uniform Matrices {
	mat4 view;
	mat4 proj;
} viewport;

layout(std140, binding = 2) uniform BrushData {	
	float pressure;
	float tilt;
	float orientation;
	float rotation;
	vec2 position;
	vec4 color;
} brush_data;
layout(binding = 1) uniform sampler2D brush_alpha;

out float v_radius;
out vec2 v_texcoord;
out vec4 v_color;

void main() {
	v_texcoord = vertices[gl_VertexID] + 0.5f;

	//Apply a kernel to get the texture outline
	const float radius = brush_data.pressure * 15.0f;
	v_radius = radius;
	v_color = brush_data.color;
	
	const vec2 position = vertices[gl_VertexID] * radius  + brush_data.position;
	gl_Position = viewport.proj * viewport.view * vec4(position, 0.0, 1.0);
}