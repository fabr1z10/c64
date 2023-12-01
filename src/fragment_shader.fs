R"(
#version 330 core

in float color;

out vec4 FragColor;

uniform sampler2D palette;

void main()
{
    //FragColor = vec4(1, 0, 0, 1);// texture(palette, float(color));
    FragColor = texture(palette, vec2(color, 0));

}

)"