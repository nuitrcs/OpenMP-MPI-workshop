program hello_mpi 
use mpi 
implicit none 

integer ierr

call MPI_INIT(ierr) 
write(*,*)  "Hello, world" 
call MPI_Finalize(ierr) 

end program hello_mpi
