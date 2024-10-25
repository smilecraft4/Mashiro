#version 420 core

out vec4 FragColor;

in vec2 v_texcoord;

layout (binding = 0) uniform sampler2D screen_tex;

void main() {
	FragColor = texture(screen_tex, v_texcoord) + vec4(v_texcoord, 0.0, 1.0);
}