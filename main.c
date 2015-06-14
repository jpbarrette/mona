#include <limits.h>

// TODO: get rid of glew on mac
#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <stdlib.h>

#include "mona.h"
#include "gl_utils.h"
#include "stb_image.h"

GLuint g_goal_display_vao = 0;
GLuint g_goal_display_vbo = 0;
GLuint g_goal_display_ebo = 0;
GLuint g_goal_display_program = 0;

GLuint vao = 0;
GLuint points_vbo = 0;
GLuint colors_vbo = 0;
GLuint shader_programme = 0;
GLuint g_rboColor = 0;
GLuint g_rboDepth = 0;
GLuint g_framebuffer = 0;
int g_width = 0;
int g_height = 0;
GLuint g_goal_texture = 0;


GLFWwindow* g_window = NULL;
unsigned char* g_goal_surface = NULL;
unsigned char* g_test_surface = NULL;


unsigned long long gl_difference()
{
    glReadPixels(0, 0, g_width, g_height, GL_RGBA, GL_UNSIGNED_BYTE, g_test_surface);

    return difference(g_width, g_height, g_test_surface, g_goal_surface);
}



void init_goal_draw()
{
    // Create Vertex Array Object
    glGenVertexArrays(1, &g_goal_display_vao);
    glBindVertexArray(g_goal_display_vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    glGenBuffers(1, &g_goal_display_vbo);

    GLfloat vertices[] = {
        //  Position  Texcoords
        -1.0f,  1.0f, 0.0f, 0.0f, // Top-left
         1.0f,  1.0f, 1.0f, 0.0f, // Top-right
         1.0f, -1.0f, 1.0f, 1.0f, // Bottom-right
        -1.0f, -1.0f, 0.0f, 1.0f  // Bottom-left
    };

    glBindBuffer(GL_ARRAY_BUFFER, g_goal_display_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create an element array 
    {
        glGenBuffers(1, &g_goal_display_ebo);
        GLuint elements[] = {
            0, 1, 2,
            2, 3, 0
        };

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_goal_display_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    }

    // TODO free up *_source

    // Create and compile the vertex shader
    int params = -1;

    const char* vertex_shader_source = read_file("goal_vs.glsl");
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    /* check for shader compile errors - very important! */
    glGetShaderiv (vertex_shader, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
        fprintf (stderr, "ERROR: GL shader index %i did not compile\n", vertex_shader);
        _print_shader_info_log (vertex_shader);
        exit(1); /* or exit or something */
    }
    _print_shader_info_log (vertex_shader);

    
    // Create and compile the fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_shader_source = read_file("goal_fs.glsl");
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    /* check for compile errors */
    glGetShaderiv (fragment_shader, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
        fprintf (stderr, "ERROR: GL shader index %i did not compile\n", fragment_shader);
        _print_shader_info_log (fragment_shader);
        exit(1); /* or exit or something */
    }
    _print_shader_info_log (fragment_shader);

    // Link the vertex and fragment shader into a shader program
    g_goal_display_program = glCreateProgram();
    glAttachShader(g_goal_display_program, vertex_shader);
    glAttachShader(g_goal_display_program, fragment_shader);
    glBindFragDataLocation(g_goal_display_program, 0, "outColor");
    glLinkProgram(g_goal_display_program);

    print_all(g_goal_display_program);

    /* check for shader linking errors - very important! */
    glGetProgramiv (g_goal_display_program, GL_LINK_STATUS, &params);
    if (GL_TRUE != params) {
        fprintf (
            stderr,
            "ERROR: could not link shader programme GL index %i\n",
            g_goal_display_program
            );
        _print_programme_info_log (g_goal_display_program);
        exit(1);
    }

    // Specify the layout of the vertex data
    GLint pos_attrib = glGetAttribLocation(g_goal_display_program, "position");
    if (pos_attrib == GL_INVALID_OPERATION)
    {
        fprintf(stderr, "Could not get 'position' attribute location");
        exit(1);
    }

    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    GLint tex_attrib = glGetAttribLocation(g_goal_display_program, "texcoord");
    if (tex_attrib == GL_INVALID_OPERATION)
    {
        fprintf(stderr, "Could not get 'texcoord' attribute location");
        exit(1);
    }
    glEnableVertexAttribArray(tex_attrib);
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    // Load textures
    {
        GLuint texture;
        glGenTextures(1, &texture);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_width, g_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_goal_surface);
        // TODO: don't understand this part
        //glUniform1i(glGetUniformLocation(g_goal_display_program, "texGoal"), 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
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

    // Frame buffer init
    {
        // Color renderbuffer.
        glGenRenderbuffers(1,&g_rboColor);
        glBindRenderbuffer(GL_RENDERBUFFER, g_rboColor);
        // Set storage for currently bound renderbuffer.
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, g_width, g_height);

        // Depth renderbuffer
        glGenRenderbuffers(1,&g_rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, g_rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, g_width, g_height);

        // Framebuffer
        glGenFramebuffers(1, &g_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, g_rboColor);
        // Set renderbuffers for currently bound framebuffer
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_rboDepth);

        // Set to write to the framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);

        // Tell glReadPixels where to read from.
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) 
        {
            fprintf(stderr, "framebuffer error:");
            switch (status) {
                case GL_FRAMEBUFFER_UNDEFINED:
                    fprintf(stderr, "GL_FRAMEBUFFER_UNDEFINED");
                break;

                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
                break;

                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
                break;

                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
                break;

                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
                break;

                case GL_FRAMEBUFFER_UNSUPPORTED:
                    fprintf(stderr, "GL_FRAMEBUFFER_UNSUPPORTED");
                break;

                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
                break;

                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
                break;

                case 0:
                    fprintf(stderr, "0");
                break;
            }
            exit(1);
        }
    }

    // TODO enable culling, but when we know that triangles met front facing requirements
    
    //glEnable (GL_CULL_FACE); // cull face
    //glCullFace (GL_BACK); // cull back face
    //glFrontFace (GL_CW); // GL_CCW for counter clock-wise
}

void draw_dna(dna_t * dna)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_framebuffer);

    glClearColor(0.5, 0.5, 0.5, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, g_width, g_height);
    
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

    glFinish();

    
    
    
    static double lastTime = 0;
    double currentTime = glfwGetTime();
    if ( currentTime - lastTime >= 1.0 )
    { 
        // If last prinf() was more than 1 sec ago
        lastTime = currentTime;

        glUseProgram(g_goal_display_program);

        glBindVertexArray (g_goal_display_vao);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, g_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glClearColor(0.5, 0.5, 0.5, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, g_width, g_height);
        glBlitFramebuffer(0, 0, g_width, g_height, 0, 0, g_width, g_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Draw a rectangle from the 2 triangles using 6 indices
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(g_window);



        // Drawing goal surface (need init_goal_draw)
        //
        /* glUseProgram(g_goal_display_program); */

        /* glBindVertexArray (g_goal_display_vao); */

        /* glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); */

        /* glClearColor(0.5, 0.5, 0.5, 1.f); */
        /* glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); */
        /* glViewport(0, 0, g_width, g_height); */

        /* // Draw a rectangle from the 2 triangles using 6 indices */
        /* glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); */

        /* glfwSwapBuffers(g_window); */

    }

    if (glfwWindowShouldClose (g_window))
    {
        glfwTerminate();
        exit(0);
    }
}

int main(int argc, char ** argv) 
{
    // Need this at the beginning of program to determine window's width and height
    g_goal_surface = load_texture("mona.png", &g_width, &g_height);
    g_test_surface = (unsigned char*)malloc(g_width * g_height * 4);

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

    g_window = glfwCreateWindow (g_width, g_height, "EvoLearning", NULL, NULL);
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
    //init_goal_draw();
    mainloop();
}

