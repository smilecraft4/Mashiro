#version 420 core

out vec4 FragColor;

in vec2 texcoord;

uniform sampler2D layer;
uniform vec3 tint;

void main() {
	FragColor = vec4(tint, 1.0) * vec4(vec3(1.0), texture(layer, texcoord).a);
}