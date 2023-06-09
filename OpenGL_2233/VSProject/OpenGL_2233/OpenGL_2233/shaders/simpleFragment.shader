#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 uv;
in vec3 normal;

uniform sampler2D mainTex;
//uniform vec3 lightPosition;

void main()
{
    FragColor = vec4(color, 1.0f) * texture(mainTex, uv);
}