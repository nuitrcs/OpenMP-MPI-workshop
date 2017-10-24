#include <stdlib.h>
#include<stdio.h>
#include <mpi.h>

int main(int argc, char **argv){
int np, rank, message;

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &np);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0){
    message = 48151623;

    MPI_Send(&message, 1, MPI_INT, 1, 0,
             MPI_COMM_WORLD);
    printf("Process %d sends %d\n",
           rank, message);
} 
else if(rank == 1){
    MPI_Recv(&message, 1, MPI_INT, 0, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d receives %d\n",
           rank, message);
}

MPI_Finalize();
}

