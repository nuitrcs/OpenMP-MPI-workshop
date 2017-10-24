#include <mpi.h>
#include <stdio.h>
int main(int argc, char **argv){
int np, rank;

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &np);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0){
printf("I am master with rank: %d\n",rank);
}
else {
printf("My rank is: %d\n",rank);
}

MPI_Finalize();
}
