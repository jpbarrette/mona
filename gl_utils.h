#ifndef __MONA_C_GL_UTILS_H__
#define __MONA_C_GL_UTILS_H__

#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library


void _print_shader_info_log (GLuint shader_index);
void _print_programme_info_log (GLuint sp);
const char* read_file(const char* filename);
unsigned char* load_texture (const char* file_name, int* width, int* height);
const char* GL_type_to_string (GLenum type);
void print_all (GLuint sp);



#endif // __MONA_C_GL_UTILS_H__
