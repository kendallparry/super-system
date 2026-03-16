#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sobel.h"
#include "rtclock.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Sobel kernels
int Kx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

int Ky[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};


// Globals: Image and threading data
unsigned char **input_image;
unsigned char **output_image;
unsigned char threshold = 127;
int width, height;
int num_threads;


/**
 * Main method
 */
int main(int argc, char *argv[]) {

    // Handle command line inputs
    if (argc < 4) {
        printf("Usage: ./sobel <input-file> <num-threads (>= 1)> <threshold (0-255)>\n");
        return -1;
    }
    num_threads = atoi(argv[2]);
    char *filename = argv[1];
    threshold = atoi(argv[3]);

    // Read image file into array a 1D array (see assignment write-up)
    unsigned char *data = stbi_load(filename, &width, &height, NULL, 1);

    //convert image 1D array into 2D array for easier processing
    input_image = (unsigned char**) malloc(sizeof(unsigned char*) * height);
    for (int i = 0; i < height; i++) {
        // assign each row the proper pixel offset
        input_image[i] = &data[i * width];
    }

    // malloc a size 'height' array of pointers (these are the rows)
    output_image = malloc(sizeof(unsigned char*) * height);

    // iterate through each row and malloc an array of size 'width'
    for (int i = 0; i < height; i++) {
        output_image[i] = malloc(sizeof(int) * width);
    }

    printf("Loaded %s. Height=%d, Width=%d\n", filename, height, width);

    // Start clocking!
    double startTime, endTime;
    startTime = rtclock();

    // Prepare and create threads
    pthread_t threads[num_threads];
    sobelChunk chunks[num_threads];
    int numRows = height/num_threads;
    int remainder = height%num_threads;
    int currentRow = 0;
    
    for (int i = 0; i<num_threads; i++){
        sobelChunk chunk;
        chunk.startRow = currentRow;
        chunk.endRow = chunk.startRow + numRows;
        if(remainder != 0){
            chunk.endRow += 1;
            remainder --;
        }
        currentRow = chunk.endRow;
        chunks[i]=chunk;

        pthread_create(&threads[i], NULL, sobelRoutine, &chunks[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i<num_threads; i++){
        pthread_join(threads[i], NULL);
    }
    
    // End clocking!
    endTime = rtclock();
    printf("Time taken (thread count = %d): %.6f sec\n", num_threads, (endTime - startTime));

    // Save the file!
    // This code re-maps output_image[][] down to a 1D array
    unsigned char *array1D = malloc(width * height * sizeof(unsigned char));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            array1D[i * width + j] = output_image[i][j];
        }
    }

    //export the 1D array as a file named <original file name>-sobel.jpg
    char* outfilename = malloc(strlen(filename)+7);
    strncpy(outfilename, filename, strlen(filename)-4);
    outfilename[strlen(filename)-4] = '\0';
    char* addition="-sobel.jpg";
    strcat(outfilename, addition);
    stbi_write_jpg(outfilename, width, height, 1, array1D, 80);

    printf("Saved as %s\n", outfilename);

    // Free allocated memory
    for (int i = 0; i< height; i++){
        free(output_image[i]);
        output_image[i]=NULL;
    }
    free(input_image);
    free(data);
    free(output_image);
    free(array1D);
    free(outfilename);

    return 0;
}
