#version 410

in vec2 Texcoord;
out vec4 outColor;

uniform sampler2D texGoal;

void main() 
{
    outColor = texture(texGoal, Texcoord);
}
