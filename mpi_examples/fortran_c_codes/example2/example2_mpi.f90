program mpi_example2
use mpi
implicit none
integer ierr, np, rank, message

call MPI_INIT(ierr)
call MPI_COMM_SIZE(MPI_COMM_WORLD, np, ierr)
call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierr)

if (rank == 0) then
    message = 48151623 

    call MPI_SEND(message, 1, MPI_INTEGER, 1, 0,&
              MPI_COMM_WORLD, ierr)

    write(*,*) "process ", rank, " sends ", message 
else if (rank == 1) then
    call MPI_RECV(message, 1, MPI_INTEGER, 0, 0,& 
              MPI_COMM_WORLD, MPI_STATUS_IGNORE, ierr)

    write(*,*) "process ",rank," receives ", message

!else if (rank == 2) then
    !write(*,*) "process ",rank," receives ", message

endif 

call MPI_Finalize(ierr) 
end program mpi_example2
