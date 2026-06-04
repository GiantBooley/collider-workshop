#version 330 core
out vec4 FragColor;

in vec2 texcoord;

uniform vec4 color;

void main() {
	if (distance(texcoord, vec2(0.5, 0.5)) > 0.5) {
		discard;
	}

	FragColor = color;
}
