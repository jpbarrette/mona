#version 410

in vec2 vp;
in vec4 vertex_color;

out vec4 color;

void main () {
    color = vertex_color;
    gl_Position = vec4(vp, 0.5, 1.0);
}
