#version 420 core

out vec4 FragColor;

in vec2 v_texcoord;

layout (binding = 0) uniform sampler2D layer;

uniform vec3 tint;

void main() {
	FragColor = texture(layer, v_texcoord);
}