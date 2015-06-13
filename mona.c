// written by nick welch <nick@incise.org>.  author disclaims copyright.


#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "mona.h"
#include "mona_draw.h"

extern void draw_dna(dna_t * dna);
extern unsigned long long gl_difference();

#define RANDINT(max) (int)((random() / (double)RAND_MAX) * (max))
#define RANDDOUBLE(max) ((random() / (double)RAND_MAX) * max)
#define RANDSIGNEDDOUBLE(max) (((random() / (double)RAND_MAX) * 2 * max) - max)
#define ABS(val) ((val) < 0 ? -(val) : (val))
#define CLAMP(val, min, max) ((val) < (min) ? (min) : \
                              (val) > (max) ? (max) : (val))



enum COLORS
{
    COLORS_R = 0,
    COLORS_G = 1,
    COLORS_B = 2,
    COLORS_A = 3
};

enum POINTS
{
    POINTS_X = 0,
    POINTS_Y = 1
};

void increment_color(dna_t* dna, unsigned idx, enum COLORS color, float increment)
{
    for (unsigned i = 0; i < NUM_POINTS; ++i)
    {
        dna->colors[idx][i][color] = CLAMP(dna->colors[idx][i][color] + increment, -1.0f, 1.0f);
    }
}

void set_color(dna_t* dna, unsigned idx, enum COLORS color, float val)
{
    for (unsigned i = 0; i < NUM_POINTS; ++i)
    {
        dna->colors[idx][i][color] = val;
    }
}

float get_color(dna_t* dna, unsigned idx, enum COLORS color)
{
    return dna->colors[idx][0][color];
}


void increment_point(dna_t* dna, unsigned idx, unsigned point_i, enum POINTS point, float increment)
{
    dna->points[idx][point_i][point] = CLAMP(dna->points[idx][point_i][point] + increment, -1.0f, 1.0f);
}

void set_point(dna_t* dna, unsigned idx, unsigned point_i, enum POINTS point, float val)
{
    dna->points[idx][point_i][point] = val;
}

void init_dna(dna_t * dna)
{
    for(int i = 0; i < NUM_SHAPES; i++)
    {
        for(int j = 0; j < NUM_POINTS; j++)
        {
            dna->points[i][j][0] = RANDSIGNEDDOUBLE(1.0f);
            dna->points[i][j][1] = RANDSIGNEDDOUBLE(1.0f);
        }
        
        set_color(dna, i, COLORS_R, RANDSIGNEDDOUBLE(1));
        set_color(dna, i, COLORS_G, RANDSIGNEDDOUBLE(1));
        set_color(dna, i, COLORS_B, RANDSIGNEDDOUBLE(1));
        set_color(dna, i, COLORS_A, RANDSIGNEDDOUBLE(1));
    }
}

void mutate(dna_t* dna_test)
{
    unsigned mutated_shape = RANDINT(NUM_SHAPES);
    double roulette = RANDDOUBLE(2.8);
    double drastic = RANDDOUBLE(2);
     
    // mutate color
    if(roulette<1)
    {
        if(get_color(dna_test, mutated_shape, COLORS_A) < 0.01 // completely transparent shapes are stupid
                || roulette<0.25)
        {
            if(drastic < 1)
                increment_color(dna_test, mutated_shape, COLORS_A, RANDSIGNEDDOUBLE(0.1));
            else
                set_color(dna_test, mutated_shape, COLORS_A, RANDDOUBLE(1.0));
        }
        else if(roulette<0.50)
        {
            if(drastic < 1)
                increment_color(dna_test, mutated_shape, COLORS_R, RANDSIGNEDDOUBLE(0.1));
            else
                set_color(dna_test, mutated_shape, COLORS_R, RANDDOUBLE(1.0));
        }
        else if(roulette<0.75)
        {
            if(drastic < 1)
                increment_color(dna_test, mutated_shape, COLORS_G, RANDSIGNEDDOUBLE(0.1));
            else
                set_color(dna_test, mutated_shape, COLORS_G, RANDDOUBLE(1.0));
        }
        else
        {
            if(drastic < 1)
                increment_color(dna_test, mutated_shape, COLORS_B, RANDSIGNEDDOUBLE(0.1));
            else
                set_color(dna_test, mutated_shape, COLORS_B, RANDDOUBLE(1.0));
        }
    }
    
    // mutate shape
    else if(roulette < 2.0)
    {
        int point_i = RANDINT(NUM_POINTS);
        if(roulette<1.5)
        {
            if(drastic < 1)
                increment_point(dna_test, mutated_shape, point_i, POINTS_X, RANDSIGNEDDOUBLE(0.1f));
            else
                set_point(dna_test, mutated_shape, point_i, POINTS_X, RANDSIGNEDDOUBLE(1.0f));
        }
        else
        {
            if(drastic < 1)
                increment_point(dna_test, mutated_shape, point_i, POINTS_Y, RANDSIGNEDDOUBLE(0.1f));
            else
                set_point(dna_test, mutated_shape, point_i, POINTS_Y, RANDSIGNEDDOUBLE(1.0f));
        }
    }

    // mutate stacking
    else
    {
        int destination = RANDINT(NUM_SHAPES);
        float s[NUM_POINTS][2];
        memcpy(&s, &dna_test->points[mutated_shape], sizeof(s));
        memcpy(&dna_test->points[mutated_shape], &dna_test->points[destination], sizeof(s));
        memcpy(&dna_test->points[destination], s, sizeof(s));
        
        float c[NUM_POINTS][4];
        memcpy(&c, &dna_test->colors[mutated_shape], sizeof(c));
        memcpy(&dna_test->colors[mutated_shape], &dna_test->colors[destination], sizeof(c));
        memcpy(&dna_test->colors[destination], c, sizeof(c));
    }

    // Could we have triangles split in half and others being removed if they participate almost nothing to the image?

}

unsigned MAX_FITNESS = UINT_MAX;

unsigned char * goal_data = NULL;

unsigned long long difference(unsigned width, unsigned height, unsigned char * test_surface, unsigned char * goal_surface)
{
    unsigned long long difference = 0;

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            int p = (y*width + x) * 4;
            
            float test_r = test_surface[p];
            float test_g = test_surface[p + 1];
            float test_b = test_surface[p + 2];
            //float test_a = test_surface[p + 3];

            float goal_r = goal_surface[p];
            float goal_g = goal_surface[p + 1];
            float goal_b = goal_surface[p + 2];
            //float goal_a = goal_surface[p + 3];
            
            //difference += (test_a - goal_a) * (test_a - goal_a);
            difference += (test_r - goal_r) * (test_r - goal_r);
            difference += (test_g - goal_g) * (test_g - goal_g);
            difference += (test_b - goal_b) * (test_b - goal_b);
        }
    }

    return difference;
}



void mainloop()
{
    struct timeval start;
    gettimeofday(&start, NULL);

    dna_t dna_best;
    dna_t dna_test;
    
    init_dna(&dna_best);
    memcpy((void *)&dna_test, (const void *)&dna_best, sizeof(dna_t));

    unsigned long long lowestdiff = ULLONG_MAX;
    int teststep = 0;
    int beststep = 0;
    for(;;) {
        mutate(&dna_test);

        draw_dna(&dna_test);

        unsigned long long diff = gl_difference();
        if(diff < lowestdiff)
        {
            beststep++;
            // test is good, copy to best
            memcpy(&dna_best, &dna_test, sizeof(dna_t));
            lowestdiff = diff;
        }
        else
        {
            // test sucks, copy best back over test
            memcpy(&dna_test, &dna_best, sizeof(dna_t));
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
    }
}


