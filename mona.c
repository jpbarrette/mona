// written by nick welch <nick@incise.org>.  author disclaims copyright.

#ifndef NUM_POINTS
#define NUM_POINTS 3
#endif

#ifndef NUM_SHAPES
#define NUM_SHAPES 40
#endif



#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>

#define RANDINT(max) (int)((random() / (double)RAND_MAX) * (max))
#define RANDDOUBLE(max) ((random() / (double)RAND_MAX) * max)
#define RANDSIGNEDDOUBLE(max) (((random() / (double)RAND_MAX) * 2 * max) - max)
#define ABS(val) ((val) < 0 ? -(val) : (val))
#define CLAMP(val, min, max) ((val) < (min) ? (min) : \
                              (val) > (max) ? (max) : (val))

int WIDTH;
int HEIGHT;
SDL_Window* gWindow = NULL;
SDL_Surface* gScreen = NULL;
//The window renderer
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;


typedef struct {
    uint8_t r, g, b, a;
    SDL_Point points[NUM_POINTS];
} shape_t;

shape_t dna_best[NUM_SHAPES];
shape_t dna_test[NUM_SHAPES];

int mutated_shape;

void draw_shape(shape_t * dna, SDL_Texture* surf, int i)
{
    shape_t * shape = &dna[i];

    SDL_SetRenderDrawColor( gRenderer, shape->r, shape->g, shape->b, shape->a);
    SDL_RenderDrawLines(gRenderer, &shape->points, NUM_POINTS);
}

void draw_dna(shape_t * dna, SDL_Texture* texture)
{
    //Make self render target
    SDL_SetRenderTarget( gRenderer, texture );
    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
    SDL_RenderClear(gRenderer);

    for(int i = 0; i < NUM_SHAPES; i++)
        draw_shape(dna, texture, i);

    SDL_RenderPresent(gRenderer);
    //Reset render target
    SDL_SetRenderTarget( gRenderer, NULL );
}

void init_dna(shape_t * dna)
{
    for(int i = 0; i < NUM_SHAPES; i++)
    {
        for(int j = 0; j < NUM_POINTS; j++)
        {
            dna[i].points[j].x = RANDDOUBLE(WIDTH);
            dna[i].points[j].y = RANDDOUBLE(HEIGHT);
        }
        dna[i].r = RANDDOUBLE(1);
        dna[i].g = RANDDOUBLE(1);
        dna[i].b = RANDDOUBLE(1);
        dna[i].a = RANDDOUBLE(1);
    }
}

int mutate(void)
{
    mutated_shape = RANDINT(NUM_SHAPES);
    double roulette = RANDDOUBLE(2.8);
    double drastic = RANDDOUBLE(2);
     
    // mutate color
    if(roulette<1)
    {
        if(dna_test[mutated_shape].a < 0.01 // completely transparent shapes are stupid
                || roulette<0.25)
        {
            if(drastic < 1)
            {
                dna_test[mutated_shape].a += RANDSIGNEDDOUBLE(0.1);
                dna_test[mutated_shape].a = CLAMP(dna_test[mutated_shape].a, 0.0, 1.0);
            }
            else
                dna_test[mutated_shape].a = RANDDOUBLE(1.0);
        }
        else if(roulette<0.50)
        {
            if(drastic < 1)
            {
                dna_test[mutated_shape].r += RANDSIGNEDDOUBLE(0.1);
                dna_test[mutated_shape].r = CLAMP(dna_test[mutated_shape].r, 0.0, 1.0);
            }
            else
                dna_test[mutated_shape].r = RANDDOUBLE(1.0);
        }
        else if(roulette<0.75)
        {
            if(drastic < 1)
            {
                dna_test[mutated_shape].g += RANDSIGNEDDOUBLE(0.1);
                dna_test[mutated_shape].g = CLAMP(dna_test[mutated_shape].g, 0.0, 1.0);
            }
            else
                dna_test[mutated_shape].g = RANDDOUBLE(1.0);
        }
        else
        {
            if(drastic < 1)
            {
                dna_test[mutated_shape].b += RANDSIGNEDDOUBLE(0.1);
                dna_test[mutated_shape].b = CLAMP(dna_test[mutated_shape].b, 0.0, 1.0);
            }
            else
                dna_test[mutated_shape].b = RANDDOUBLE(1.0);
        }
    }
    
    // mutate shape
    else if(roulette < 2.0)
    {
        int point_i = RANDINT(NUM_POINTS);
        if(roulette<1.5)
        {
            if(drastic < 1)
            {
                dna_test[mutated_shape].points[point_i].x += (int)RANDSIGNEDDOUBLE(WIDTH/10.0);
                dna_test[mutated_shape].points[point_i].x = CLAMP(dna_test[mutated_shape].points[point_i].x, 0, WIDTH-1);
            }
            else
                dna_test[mutated_shape].points[point_i].x = RANDDOUBLE(WIDTH);
        }
        else
        {
            if(drastic < 1)
            {
                dna_test[mutated_shape].points[point_i].y += (int)RANDSIGNEDDOUBLE(HEIGHT/10.0);
                dna_test[mutated_shape].points[point_i].y = CLAMP(dna_test[mutated_shape].points[point_i].y, 0, HEIGHT-1);
            }
            else
                dna_test[mutated_shape].points[point_i].y = RANDDOUBLE(HEIGHT);
        }
    }

    // mutate stacking
    else
    {
        int destination = RANDINT(NUM_SHAPES);
        shape_t s = dna_test[mutated_shape];
        dna_test[mutated_shape] = dna_test[destination];
        dna_test[destination] = s;
        return destination;
    }
    return -1;

    // Could we have triangles split in half and others being removed if they participate almost nothing to the image?

}

unsigned MAX_FITNESS = UINT_MAX;

unsigned char * goal_data = NULL;

// get Uint32 variable (pixel) from surface at x, y
Uint32 get_pixel32(SDL_Surface *surface, int x)
{
	Uint32 *pixels = (Uint32 *)surface->pixels;
	return pixels[x];
}

unsigned long long difference(SDL_Texture* test_texture)
{
    unsigned long long difference = 0;
    static SDL_Surface* test_surf = NULL;
    static SDL_Surface* goal_surf = NULL;

    if (goal_surf == NULL)
    {
        Uint32 format;
        if (SDL_QueryTexture(test_texture, &format, NULL, NULL, NULL) != 0)
        {
            printf("Error when querying texture! SDL Error:%s\n", SDL_GetError());
            return 0;
        }
        if (format == SDL_PIXELFORMAT_ARGB8888)
        {
            void* pixels = NULL;
            int pitch = 0;
            if (SDL_LockTexture(gTexture, NULL, &pixels, &pitch) != 0)
            {
                printf("Error when locking texture! SDL Error:%s\n", SDL_GetError());
                return 0;
            }
            
            goal_surf = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
            memcpy(pixels, goal_surf->pixels, pitch * goal_surf->h);
            
            
            //Unlock texture to update
            SDL_UnlockTexture(gTexture);
        }
    }


    //Lock texture for manipulation
    {
        void* pixels = NULL;
        int pitch = 0;
        SDL_LockTexture(test_texture, NULL, &pixels, &pitch);

        if (test_surf == NULL)
        {
            test_surf = SDL_CreateRGBSurfaceFrom(
                pixels, WIDTH, HEIGHT, 
                4, pitch,
                0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
        }
        else
        {
            //Copy loaded/formatted surface pixels
            memcpy(pixels, test_surf->pixels, test_surf->pitch * test_surf->h);
        }
    
        //Unlock texture to update
        SDL_UnlockTexture(test_texture);
    }


    for(int y = 0; y < HEIGHT; y++)
    {
        for(int x = 0; x < WIDTH; x++)
        {
            int thispixel = y*WIDTH + x;
            
            Uint32 tp32 = get_pixel32(test_surf, thispixel);
            Uint8 test_a, test_r, test_g, test_b;
            SDL_GetRGBA(tp32, test_surf->format, &test_r, &test_g, &test_b, &test_a);

            Uint32 gp32 = get_pixel32(goal_surf, thispixel);
            Uint8 goal_a, goal_r, goal_g, goal_b;
            SDL_GetRGBA(gp32, goal_surf->format, &goal_r, &goal_g, &goal_b, &goal_a);

            difference += (test_a - goal_a) * (test_a - goal_a);
            difference += (test_r - goal_r) * (test_r - goal_r);
            difference += (test_g - goal_g) * (test_g - goal_g);
            difference += (test_b - goal_b) * (test_b - goal_b);
        }
    }

    return difference;
}

static void mainloop()
{
    struct timeval start;
    gettimeofday(&start, NULL);

    init_dna(dna_best);
    memcpy((void *)dna_test, (const void *)dna_best, sizeof(shape_t) * NUM_SHAPES);

    SDL_Texture* test_texture = SDL_CreateTexture(gRenderer, SDL_GetWindowPixelFormat(gWindow), SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    unsigned long long lowestdiff = ULLONG_MAX;
    int teststep = 0;
    int beststep = 0;
    for(;;) {
        int other_mutated = mutate();
        draw_dna(dna_test, test_texture);

        unsigned long long diff = difference(test_texture);
        if(diff < lowestdiff)
        {
            beststep++;
            // test is good, copy to best
            dna_best[mutated_shape] = dna_test[mutated_shape];
            if(other_mutated >= 0)
                dna_best[other_mutated] = dna_test[other_mutated];
            lowestdiff = diff;
        }
        else
        {
            // test sucks, copy best back over test
            dna_test[mutated_shape] = dna_best[mutated_shape];
            if(other_mutated >= 0)
                dna_test[other_mutated] = dna_best[other_mutated];
        }

        teststep++;

#ifdef TIMELIMIT
        struct timeval t;
        gettimeofday(&t, NULL);
        if(t.tv_sec - start.tv_sec > TIMELIMIT)
        {
            printf("%0.6f\n", ((MAX_FITNESS-lowestdiff) / (float)MAX_FITNESS)*100);
#ifdef DUMP
            char filename[50];
            sprintf(filename, "%d.data", getpid());
            FILE * f = fopen(filename, "w");
            fwrite(dna_best, sizeof(shape_t), NUM_SHAPES, f);
            fclose(f);
#endif
            return;
        }
#else
        if(teststep % 100 == 0)
            printf("Step = %d/%d\nFitness = %0.6f%%\n",
                    beststep, teststep, ((MAX_FITNESS-lowestdiff) / (float)MAX_FITNESS)*100);
#endif

        if(teststep % 100 == 0)
        {
            //Initialize renderer color
            SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

            //Clear screen
            SDL_RenderClear( gRenderer );
            
            //Render texture to screen
            SDL_RenderCopy( gRenderer, test_texture, NULL, NULL );
            
            //Update screen
            SDL_RenderPresent( gRenderer );
            
            
        }
    }
}

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int main(int argc, char ** argv) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }	// initialize SDL
    
    //Initialize PNG loading
    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) )
    {
        printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
        return 1;
    }


    //Create window
    gWindow = SDL_CreateWindow( "EvoLisa", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( gWindow == NULL )
    {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        return 1;
    }

    gRenderer = SDL_CreateRenderer( gWindow, -1, 0);
    if( gRenderer == NULL )
    {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return 1;
    }

    int numdrivers = SDL_GetNumRenderDrivers (); 
    printf("Render driver count: \n");
    for (int i=0; i<numdrivers; i++) { 
        SDL_RendererInfo drinfo; 
        SDL_GetRenderDriverInfo (i, &drinfo); 
        printf("Driver name (%s): ", drinfo.name);
        if (drinfo.flags & SDL_RENDERER_SOFTWARE) printf(" the renderer is a software fallback\n");
        if (drinfo.flags & SDL_RENDERER_ACCELERATED) printf(" the renderer uses hardware acceleration\n");
        if (drinfo.flags & SDL_RENDERER_PRESENTVSYNC) printf(" present is synchronized with the refresh rate\n");
        if (drinfo.flags & SDL_RENDERER_TARGETTEXTURE) printf(" the renderer supports rendering to texture\n");
    } 

    
    // TODO delete surface 
    SDL_Surface *image = NULL;	// loaded image
    const char * filename = (argc == 1 ? "portrait.png" : argv[1]);
    image = IMG_Load(filename);	// open our file
    if (image == NULL)
    {
        printf( "Image could not be loaded! SDL Error: %s\n", IMG_GetError() );
        return 1;
    }
    
    WIDTH = image->w;
    HEIGHT = image->h;

    //Create texture from surface pixels
    gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if( gTexture == NULL )
    {
        printf( "Unable to create texture from %s! SDL Error: %s\n", filename, SDL_GetError() );
    }
    SDL_UpdateTexture(gTexture, NULL, image->pixels, image->pitch);
    SDL_FreeSurface(image);

    srandom(getpid() + time(NULL));
    mainloop();
    
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

