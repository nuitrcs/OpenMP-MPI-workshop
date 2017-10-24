PROGRAM mpilayout
USE MPI ! f90 designation
! include "mpif.h" ! f77 designation
integer ierr


CALL MPI_INIT(ierr)


CALL MPI_FINALIZE(ierr)


END PROGRAM mpilayout
