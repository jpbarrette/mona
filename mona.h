#ifndef __MONA_C_MONA_H__
#define __MONA_C_MONA_H__

#ifndef NUM_POINTS
#define NUM_POINTS 3
#endif

#ifndef NUM_SHAPES
#define NUM_SHAPES 40
#endif

typedef struct 
{
    // We have structures of array for vertex object buffers
    float points[NUM_SHAPES][NUM_POINTS][2];
    float colors[NUM_SHAPES][NUM_POINTS][4];
} dna_t;

void mainloop();

#endif // __MONA_C_MONA_H__
