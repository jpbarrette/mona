#include <limits.h>

// TODO: get rid of glew on mac
#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <stdlib.h>

#include "mona.h"
#include "stb_image.h"

GLuint vao = 0;
GLuint points_vbo = 0;
GLuint colors_vbo = 0;
GLuint shader_programme = 0;
GLFWwindow* g_window = NULL;
unsigned char* g_goal_surface = NULL;

/* print errors in shader compilation */
void _print_shader_info_log (GLuint shader_index) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog (shader_index, max_length, &actual_length, log);
	printf ("shader info log for GL index %i:\n%s\n", shader_index, log);
}

/* print errors in shader linking */
void _print_programme_info_log (GLuint sp) {
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
    int force_channels = 4;
    unsigned char* image_data = (unsigned char*)stbi_loadf (file_name, width, height, &n, force_channels);
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

    return image_data;
}

void gl_difference()
{

}

void init_draw()
{
    int params = -1;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &points_vbo);
    glGenBuffers(1, &colors_vbo);
        
    {
        const char* vertex_shader = read_file("test_vs.glsl");
        const char* fragment_shader = read_file("test_fs.glsl");
        
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertex_shader, NULL);
        glCompileShader(vs);

	/* check for shader compile errors - very important! */
	glGetShaderiv (vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
            fprintf (stderr, "ERROR: GL shader index %i did not compile\n", vs);
            _print_shader_info_log (vs);
            exit(1); /* or exit or something */
	}

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragment_shader, NULL);
        glCompileShader(fs);

	/* check for compile errors */
	glGetShaderiv (fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
            fprintf (stderr, "ERROR: GL shader index %i did not compile\n", fs);
            _print_shader_info_log (fs);
            exit(1); /* or exit or something */
	}
        
        shader_programme = glCreateProgram();
        glAttachShader(shader_programme, fs);
        glAttachShader(shader_programme, vs);
        glLinkProgram(shader_programme);

	/* check for shader linking errors - very important! */
	glGetProgramiv (shader_programme, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
            fprintf (
                stderr,
                "ERROR: could not link shader programme GL index %i\n",
                shader_programme
		);
            _print_programme_info_log (shader_programme);
            exit(1);
	}

    }

    glEnable (GL_CULL_FACE); // cull face
    glCullFace (GL_BACK); // cull back face
    glFrontFace (GL_CW); // GL_CCW for counter clock-wise
}

void draw_dna(dna_t * dna)
{
    glClearColor(0.5, 0.5, 0.5, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glViewport (0, 0, WIDTH, HEIGHT);
    
    glUseProgram(shader_programme);
    
    glBindVertexArray (vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*NUM_POINTS*NUM_SHAPES*2, &dna->points, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0); // Specify how the data for position can be accessed
    glEnableVertexAttribArray(0);
        
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*NUM_POINTS*NUM_SHAPES*4, &dna->colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0); // Specify how the data for color can be accessed
    glEnableVertexAttribArray(1);
        
    glfwPollEvents();
    if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose (g_window, 1);
    }
    // put the stuff we've been drawing onto the display
    glDrawArrays (GL_TRIANGLES, 0, NUM_SHAPES*NUM_POINTS);

    glfwSwapBuffers(g_window);

    if (glfwWindowShouldClose (g_window))
    {
        glfwTerminate();
        exit(0);
    }
}

int main(int argc, char ** argv) 
{
    int width = 0;
    int height = 0;
    g_goal_surface = load_texture("portrait.png", &width, &height);

    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit ()) 
    {
        fprintf (stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

#ifdef __APPLE__
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint (GLFW_SAMPLES, 4);
#endif // __APPLE__

    g_window = glfwCreateWindow (width, height, "EvoLearning", NULL, NULL);
    if (!g_window) {
        fprintf (stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(g_window);
  
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

    // get version info
    const GLubyte* renderer = glGetString (GL_RENDERER);
    const GLubyte* version = glGetString (GL_VERSION);
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable (GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"

    init_draw();
    mainloop();
}

