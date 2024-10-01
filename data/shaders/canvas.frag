#version 460 core
out vec4 FragColor;

in vec2 v_position;
in vec2 v_texcoord;

uniform sampler2D canvas_background;
uniform sampler2D canvas_foreground;

void main() {
	vec4 background = texture(canvas_background, v_texcoord);
	vec4 foreground = texture(canvas_foreground, v_texcoord);
	float opacity = foreground.z;

	FragColor = mix(background, foreground, opacity);
}