#version 420 core

out vec4 FragColor;

in float v_radius;
in vec2 v_texcoord;
in vec4 v_color;

layout (binding=1) uniform sampler2D brush_alpha;

float sdCircle(vec2 p, float r)
{
    return length(p) - r;
}

void main() {
	FragColor = mix(vec4(v_color.rgb, 0.0), v_color, sdCircle(v_texcoord - vec2(0.5), v_radius));
}