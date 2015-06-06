#include "mona.h"

GLuint vao = 0;
GLuint points_vbo = 0;
GLuint colors_vbo = 0;
GLuint shader_programme = 0;


void init_draw(dna_t* dna)
{
    glGenVertexArrays (1, &first_string_vao);
    glGenBuffers(1, &points_vbo);
    glGenBuffers(1, &colors_vbo);

    {
        const char* vertex_shader = read_file("test_vs.glsl");
        const char* fragment_shader = read_file("test_fs.glsl");
        
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertex_shader, NULL);
        glCompileShader(vs);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragment_shader, NULL);
        glCompileShader(fs);
        
        shader_programme = glCreateProgram();
        glAttachShader(shader_programme, fs);
        glAttachShader(shader_programme, vs);
        glLinkProgram(shader_programme);
    }

}

void draw_dna(dna_t * dna)
{
    glUseProgram(shader_programme);

    glBindVertexArray (vao);

    glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat*NUM_POINTS*NUM_SHAPES*2), &dna->points, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat*NUM_POINTS*NUM_SHAPES*4), &dna->colors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);

    glDrawArrays (GL_TRIANGLES, 0, NUM_SHAPES*NUM_POINTS);

    glfwSwapBuffers(window);
}
