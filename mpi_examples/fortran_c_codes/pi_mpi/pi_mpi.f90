PROGRAM pi_mpi 
use mpi 
double precision mypi, pi, h, x, f, a
integer n, myid, numprocs, i, ierr
 
call MPI_INIT( ierr )
call MPI_COMM_RANK( MPI_COMM_WORLD, myid, ierr )
call MPI_COMM_SIZE( MPI_COMM_WORLD, numprocs, ierr )

if (myid .eq. 0) n = 900000000

call MPI_BCAST(n,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
    
h = 1.0D+0 / REAL (n) ! trapezoid base
mypi = 0.0D+0

do i = myid+1, n, numprocs ! cyclic distribution
    x = h * ( REAL (i) - 0.5D+0)
    mypi = mypi + 4.0D+0 / (1.0D+0 + x * x)
end do
!!!!!collect partial sums!!!!!!!!!!!!!!!!
call MPI_REDUCE(mypi, pi, 1, MPI_REAL8, & 
    MPI_SUM, 0, MPI_COMM_WORLD,ierr)

if (myid .eq. 0) then
    pi=pi*h
    write(*,*) pi
end if

call MPI_FINALIZE(ierr)
end program pi_mpi

