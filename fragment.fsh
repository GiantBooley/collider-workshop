#version 330 core
out vec4 FragColor;

in vec2 texcoord;

uniform sampler2D texture;

void main() {
	vec4 color = texture2D(texture, texcoord);
	FragColor = color;
}
