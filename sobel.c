#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include "sobel.h"

// pthread routine to call the sobelfilter function
void *sobelRoutine(void *arg){
    sobelChunk *chunk = (sobelChunk*)arg;
    int startRow = chunk->startRow;
    int endRow = chunk->endRow;

    sobelFilter(startRow, endRow);

    return NULL;
}

// main sobel filter function for a range of rows
void sobelFilter(int startRow, int endRow){
    for (int i = startRow; i< endRow; i++){
        for(int j = 0; j<width; j++){
            if (i == 0 || j == 0 || i == height -1 || j == width-1){
                output_image[i][j] = BLACK;
            }
            else{
                //do horizontal
                int Gx=(
                    (Kx[0][0]*input_image[i-1][j-1]) + (Kx[0][1]*input_image[i-1][j]) + (Kx[0][2]*input_image[i-1][j+1]) +
                    (Kx[1][0]*input_image[i][j-1]) + (Kx[1][1]*input_image[i][j]) + (Kx[1][2]*input_image[i][j+1]) +
                    (Kx[2][0]*input_image[i+1][j-1]) + (Kx[2][1]*input_image[i+1][j]) + (Kx[2][2]*input_image[i+1][j+1])
                );
                int Gy = (
                    (Ky[0][0]*input_image[i-1][j-1]) + (Ky[0][1]*input_image[i-1][j]) + (Ky[0][2]*input_image[i-1][j+1]) +
                    (Ky[1][0]*input_image[i][j-1]) + (Ky[1][1]*input_image[i][j]) + (Ky[1][2]*input_image[i][j+1]) +
                    (Ky[2][0]*input_image[i+1][j-1]) + (Ky[2][1]*input_image[i+1][j]) + (Ky[2][2]*input_image[i+1][j+1])
                );
                //determine gradient and clamp if needed
                int G = sqrt((pow(Gx, 2.0) + pow(Gy, 2.0)));
                if (G > WHITE){
                    output_image[i][j]=WHITE;
                }
                else{
                    output_image[i][j]=G;
                }
                //threshold gradient
                if (output_image[i][j] < threshold){
                    output_image[i][j] = BLACK;
                }
            }
        }
    }
}