#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

int generate_random_number(int);
bool in_bounds(int*, int);

int main(int argc, char** argv)
{
    int n_cpus, rank;
    char processor_name;
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &n_cpus);
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    //setup code
    const int canvas_size = 1000000;
    const float pi = 3.14;
    const float epsilon = 0.01;
    int N = 0, M = 0;
    srand(rank+1);

    int index;
    for(index = 0; index < 10000; index++){
        printf("loop %d\n", index);
        if (rank == 0) {
            int coordinates[2];
            int i;
            for (i = 1; i < n_cpus; i++) {
                //receive message
                MPI_Recv(coordinates, 2, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("[%d]-> x:%d, y:%d\n", rank, coordinates[0], coordinates[1]);
                //check if in bounds
                if (in_bounds(coordinates, canvas_size)) {
                    //if yes, add to M
                    printf("[%d]-> ***was in bounds***\n", rank);
                    M++;
                }
                else
                {
                    printf("[%d]-> not in bounds\n", rank);
                }                
                //add to N
                N++;
                //check convergence
                printf("M:%.2f, N: %.2f", (float)M, (float)N);
                float convergence = 4 * ((float)M / (float)N);
                printf("[%d]-> convergence: %.2f\n", rank, convergence);

                //compare to epsilon
                bool has_converged = (float)fabs((pi - convergence)) < epsilon;
                printf("[%d]-> has_convergedf float: %.2f\n", rank, (float)fabs((pi - convergence)));
                printf("[%d]-> has_converged: %d\n", rank, has_converged);

                //if criteria met, abort all, break loop
                if (has_converged) {
                    printf("terminating\n");
                    // Finalize the MPI environment.
                    MPI_Finalize();
                    return 0;
                }
            }
        }
        else {
            //all other cores
            //pick random number x, y
            int x = generate_random_number(canvas_size);
            int y = generate_random_number(canvas_size);
            int coordinates[2] = {x, y};

            //send data
            printf("[%d]-> x:%d, y:%d\n", rank, x, y);
            MPI_Send(coordinates, 2, MPI_INT, 0, rank, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}

int generate_random_number(int canvas_size){return rand()%canvas_size;}

bool in_bounds(int* coordinates, int canvas_size) {
    long x_center = canvas_size/2;
    long y_center = x_center;
    int64_t radius_squared = ((int64_t)canvas_size / 2) * ((int64_t)canvas_size / 2);

    int64_t distance = ((coordinates[0] - x_center) * (coordinates[0] - x_center)) + ((coordinates[1] - y_center) * (coordinates[1] - y_center));
    printf("inbounds -> canvassize: %d\n", canvas_size);

    printf("inbounds -> distance: %lu\n", distance);
    printf("inbounds -> radius_squared: %lu\n", radius_squared);
    printf("inbounds -> result: %d\n", distance<radius_squared);
    return distance < radius_squared;
}