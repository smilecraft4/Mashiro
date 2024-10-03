#version 460 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

out vec2 v_position;
out vec2 v_texcoord;

uniform mat4 MV;

uniform vec2 screen_size;
uniform vec2 tile_size;

void main() {
	v_position = position;
	v_texcoord = texcoord;

	gl_Position = MV * vec4(position, 0.0, 1.0);
}