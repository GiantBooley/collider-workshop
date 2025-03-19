#version 330 core
out vec4 FragColor;

in vec2 texcoord;

uniform sampler2D texture;
uniform vec4 bounds;

void main() {
	vec2 uv = vec2(mix(bounds[0], bounds[2], texcoord.x), mix(bounds[1], bounds[3], texcoord.y));
	vec4 color = texture2D(texture, uv);
	FragColor = color;
}
