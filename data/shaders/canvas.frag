#version 420 core

out vec4 FragColor;

in vec2 texcoord;

uniform sampler2D layer;
uniform vec3 tint;

void main() {
	FragColor = texture(layer, texcoord);
}