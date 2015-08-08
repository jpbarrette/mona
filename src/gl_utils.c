#include "gl_utils.h"

#include <stdio.h>
#include <stdlib.h>

#include "stb_image.h"

/* print errors in shader compilation */
void _print_shader_info_log (GLuint shader_index) 
{
    int max_length = 2048;
    int actual_length = 0;
    char log[2048];
    glGetShaderInfoLog (shader_index, max_length, &actual_length, log);
    printf ("shader info log for GL index %i:\n%s\n", shader_index, log);
}

/* print errors in shader linking */
void _print_programme_info_log (GLuint sp) 
{
    int max_length = 2048;
    int actual_length = 0;
    char log[2048];
    glGetProgramInfoLog (sp, max_length, &actual_length, log);
    printf ("program info log for GL index %i:\n%s", sp, log);
}

const char* read_file(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp) perror("Can't open file"), exit(1);

    fseek(fp , 0L , SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    void* buffer = calloc(1, size + 1);
    if(NULL == buffer) fclose(fp), fputs("memory alloc fails",stderr),exit(1);    
    
    if(1 != fread(buffer, size, 1, fp)) fclose(fp), free(buffer), fputs("file read fails",stderr), exit(1);

    fclose(fp);
    return (const char*)buffer;
}

unsigned char* load_texture (const char* file_name, int* width, int* height)
{
    int n;

    // TODO: use SOIL instead?
    unsigned char* image_data = (unsigned char*)stbi_load(file_name, width, height, &n, STBI_rgb_alpha);
    if (!image_data) {
        fprintf (stderr, "ERROR: could not load %s\n", file_name);
        return NULL;
    }

    int width_in_bytes = *width * 4;
    unsigned char *top = NULL;
    unsigned char *bottom = NULL;
    unsigned char temp = 0;
    int half_height = *height / 2;
    
    for (int row = 0; row < half_height; row++) {
        top = image_data + row * width_in_bytes;
        bottom = image_data + (*height - row - 1) * width_in_bytes;
        for (int col = 0; col < width_in_bytes; col++) {
            temp = *top;
            *top = *bottom;
            *bottom = temp;
            top++;
            bottom++;
        }
    }

    // TODO delete image_data
    return image_data;
}

const char* GL_type_to_string (GLenum type) 
{
  switch (type) 
  {
    case GL_BOOL: return "bool";
    case GL_INT: return "int";
    case GL_FLOAT: return "float";
    case GL_FLOAT_VEC2: return "vec2";
    case GL_FLOAT_VEC3: return "vec3";
    case GL_FLOAT_VEC4: return "vec4";
    case GL_FLOAT_MAT2: return "mat2";
    case GL_FLOAT_MAT3: return "mat3";
    case GL_FLOAT_MAT4: return "mat4";
    case GL_SAMPLER_2D: return "sampler2D";
    case GL_SAMPLER_3D: return "sampler3D";
    case GL_SAMPLER_CUBE: return "samplerCube";
    case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
    default: break;
  }
  return "other";
}


/* print absolutely everything about a shader - only useful if you get really
stuck wondering why a shader isn't working properly */
void print_all (GLuint sp) 
{
	int params = -1;
	int i;

	printf ("--------------------\nshader programme %i info:\n", sp);
	glGetProgramiv (sp, GL_LINK_STATUS, &params);
	printf ("GL_LINK_STATUS = %i\n", params);
	
	glGetProgramiv (sp, GL_ATTACHED_SHADERS, &params);
	printf ("GL_ATTACHED_SHADERS = %i\n", params);
	
	glGetProgramiv (sp, GL_ACTIVE_ATTRIBUTES, &params);
	printf ("GL_ACTIVE_ATTRIBUTES = %i\n", params);
	
	for (i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveAttrib (sp, i, max_length, &actual_length, &size, &type, name);
		if (size > 1) {
			int j;
			for (j = 0; j < size; j++) {
				char long_name[64];
				int location;

				sprintf (long_name, "%s[%i]", name, j);
				location = glGetAttribLocation (sp, long_name);
				printf ("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string (type), long_name, location);
			}
		} else {
			int location = glGetAttribLocation (sp, name);
			printf ("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string (type), name, location);
		}
	}
	
	glGetProgramiv (sp, GL_ACTIVE_UNIFORMS, &params);
	printf ("GL_ACTIVE_UNIFORMS = %i\n", params);
	for (i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveUniform (sp, i, max_length, &actual_length, &size, &type, name);
		if (size > 1) {
			int j;
			for (j = 0; j < size; j++) {
				char long_name[64];
				int location;

				sprintf (long_name, "%s[%i]", name, j);
				location = glGetUniformLocation (sp, long_name);
				printf ("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string (type), long_name, location);
			}
		} else {
			int location = glGetUniformLocation (sp, name);
			printf ("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string (type), name, location);
		}
	}
	
	_print_programme_info_log (sp);
}
