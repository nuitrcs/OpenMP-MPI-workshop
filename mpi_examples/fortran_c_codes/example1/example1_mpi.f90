program mpi_example1 
use mpi 
implicit none 
integer ierr, np, rank, inum

call MPI_INIT(ierr) 
call MPI_COMM_SIZE(MPI_COMM_WORLD, np, ierr)
call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierr) 

if (rank == 0) then
write(*,*) 'I am master with rank ',rank
else 
write(*,*) 'My rank is: ',rank
endif 

if (rank == 0) write(*,*) np

call MPI_Finalize(ierr) 

end program mpi_example1
