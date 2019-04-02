#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

int generate_random_number(int);
bool in_bounds(const int*, int);

int main(int argc, char** argv)
{
    int n_cpus, rank;
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &n_cpus);
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    //setup code
    const int canvas_size = 1000000;
    const float epsilon = 0.00001;
    int N = 0, M = 0;
    float previous_value = 0;
    srand(rank + 1);

    //run near infinite time until convergence
    int index;
    for(index=0;index<100000;index++) {
        if (rank == 0) {
            int values[2];

            int i;
            for (i = 1; i < n_cpus; i++) {
                index++;
                printf("loop number %d\n", index);            
                //receive message
                MPI_Recv(values, 2, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                //add to m and n total
                M += values[0];
                N += values[1];

                //check convergence
                printf("M:%.1f, N: %.1f", (float)M, (float)N);
                float convergence = 4.0 * ((float)M / (float)N);
                printf("[%d]-> convergence: %.6f\n", rank, convergence);

                //compare to epsilon
                bool has_converged = (float)fabs((previous_value - convergence)) < epsilon;
                printf("[%d]-> has_converged float: %.6f\n", rank, (float)fabs((previous_value - convergence)));
                printf("[%d]-> has_converged: %d\n", rank, has_converged);

                //if criteria met, abort all, break loop
                if (has_converged && index > 100) {
                    printf("terminating\n");
                    // Finalize the MPI environment.
                    MPI_Finalize();
                    return 0;
                }
                else {
                    previous_value = convergence;
                    printf("previous: %f\n", previous_value);
                }
            }
        }
        else {
            //all other cores
            //pick random number x, y
            int x = generate_random_number(canvas_size);
            int y = generate_random_number(canvas_size);
            int coordinates[2] = {x, y};

            //if yes, add to M
            if (in_bounds(coordinates, canvas_size)) M++;
            //add to N
            N++;

            //send data
            int values[2] = {M, N};
            MPI_Send(values, 2, MPI_INT, 0, rank, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}

int generate_random_number(int canvas_size){return rand()%canvas_size;}

bool in_bounds(const int* coordinates, int canvas_size) {
    long x_center = canvas_size/2;
    long y_center = x_center;
    int64_t radius_squared = pow((canvas_size / 2.0), 2);
    int64_t distance_squared = (pow((coordinates[0] - x_center), 2)) + (pow((coordinates[1] - y_center), 2));

    return distance_squared < radius_squared;
}