#version 420 core

out vec4 FragColor;

in vec2 texcoord;

void main() {
	FragColor = vec4(texcoord, 0.0, 1.0);
}