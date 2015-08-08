#version 410

in vec4 color;
out vec4 frag_colour;

void main() {
    //frag_colour = vec4 (0.7, 0.1, 0.5, 1.0);
    frag_colour = color;
}
