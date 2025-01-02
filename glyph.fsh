#version 330 core
in vec2 texcoord;
out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, vec2(texcoord.x, 1. - texcoord.y)).r);
    color = textColor * sampled;
}
