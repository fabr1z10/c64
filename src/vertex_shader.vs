R"(
#version 330 core

layout (location = 0) in vec2 vPosition;
layout (location = 1) in float vColor;

uniform mat4 projection;
out float color;

void main() {
    gl_Position = projection * vec4(vPosition, 0, 1);
    color = vColor;
}

)"