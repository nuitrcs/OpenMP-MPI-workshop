heated_plate_openmp.f90                                                                             0000644 1322746 1322746 00000015714 13054131615 014272  0                                                                                                    ustar   ppk652                          ppk652                                                                                                                                                                                                                 program main

!*****************************************************************************80
!
!! MAIN is the main program for HEATED_PLATE_OPENMP.
!
!  Discussion:
!
!    This code solves the steady state heat equation on a rectangular region.
!
!    The sequential version of this program needs approximately
!    18/eps iterations to complete. 
!
!
!    The physical region, and the boundary conditions, are suggested
!    by this diagram;
!
!                   W = 0
!             +------------------+
!             |                  |
!    W = 100  |                  | W = 100
!             |                  |
!             +------------------+
!                   W = 100
!
!    The region is covered with a grid of M by N nodes, and an N by N
!    array W is used to record the temperature.  The correspondence between
!    array indices and locations in the region is suggested by giving the
!    indices of the four corners:
!
!                  I = 0
!          [0][0]-------------[0][N-1]
!             |                  |
!      J = 0  |                  |  J = N-1
!             |                  |
!        [M-1][0]-----------[M-1][N-1]
!                  I = M-1
!
!    The steady state solution to the discrete heat equation satisfies the
!    following condition at an interior grid point:
!
!      W[Central] = (1/4) * ( W[North] + W[South] + W[East] + W[West] )
!
!    where "Central" is the index of the grid point, "North" is the index
!    of its immediate neighbor to the "north", and so on.
!   
!    Given an approximate solution of the steady state heat equation, a
!    "better" solution is given by replacing each interior point by the
!    average of its 4 neighbors - in other words, by using the condition
!    as an ASSIGNMENT statement:
!
!      W[Central]  <=  (1/4) * ( W[North] + W[South] + W[East] + W[West] )
!
!    If this process is repeated often enough, the difference between 
!    successive estimates of the solution will go to zero.
!
!    This program carries out such an iteration, using a tolerance specified by
!    the user, and writes the final estimate of the solution to a file that can
!    be used for graphic processing.
!
!  Licensing:
!
!    This code is distributed under the GNU LGPL license. 
!
!  Modified:
!
!    18 October 2011
!
!  Author:
!
!    Original FORTRAN90 version by Michael Quinn.
!    This version by John Burkardt.
!
!  Reference:
!
!    Michael Quinn,
!    Parallel Programming in C with MPI and OpenMP,
!    McGraw-Hill, 2004,
!    ISBN13: 978-0071232654,
!    LC: QA76.73.C15.Q55.
!
!  Local parameters:
!
!    Local, real ( kind = 8 ) DIFF, the norm of the change in the solution from 
!    one iteration to the next.
!
!    Local, real ( kind = 8 ) MEAN, the average of the boundary values, used 
!    to initialize the values of the solution in the interior.
!
!    Local, real ( kind = 8 ) U(M,N), the solution at the previous iteration.
!
!    Local, real ( kind = 8 ) W(M,N), the solution computed at the latest 
!    iteration.
!
  use omp_lib

  implicit none

  integer ( kind = 4 ), parameter :: m = 600
  integer ( kind = 4 ), parameter :: n = 600

  real ( kind = 8 ) diff
  real ( kind = 8 ) :: eps = 0.001D+00
  integer ( kind = 4 ) i
  integer ( kind = 4 ) iterations
  integer ( kind = 4 ) iterations_print
  integer ( kind = 4 ) j
  real ( kind = 8 ) mean
  real ( kind = 8 ) u(m,n)
  real ( kind = 8 ) w(m,n)
  real ( kind = 8 ) wtime
  real ( kind = 8 ) Dutch_wind_eta
  character(len=255) :: cpuaffinity

  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) 'HEATED_PLATE_OPENMP'
  write ( *, '(a)' ) '  FORTRAN90 version'
  write ( *, '(a)' ) &
    '  A program to solve for the steady state temperature distribution'
  write ( *, '(a)' ) '  over a rectangular plate.'
  write ( *, '(a)' ) ' '
  write ( *, '(a,i8,a,i8,a)' ) '  Spatial grid of ', m, ' by ', n, ' points.'
  write ( *, '(a,g14.6)' ) &
    '  The iteration will repeat until the change is <= ', eps
  write ( *, '(a,i8)' ) &
    '  The number of processors available = ', omp_get_num_procs ( )
  write ( *, '(a,i8)' ) &
    '  The number of threads available    = ', omp_get_max_threads ( )

  Dutch_wind_eta = 1.0D0
  write (*,*) Dutch_wind_eta 
  call get_environment_variable("GOMP_CPU_AFFINITY",cpuaffinity)
  write(*,*) trim(cpuaffinity)
!
!  Set the boundary values, which don't change.
!
!  OpenMP Note:
!  You CANNOT set MEAN to zero inside the parallel region.
!
  mean = 0.0D+00
!
!$omp parallel shared ( w ) private ( i, j ) 

  !$omp do
  do i = 2, m - 1
    w(i,1) = 100.0D+00
    w(i,n) = 100.0D+00
  end do
  !$omp end do

  !$omp do
  do j = 1, n
    w(m,j) = 100.0D+00
    w(1,j) =   0.0D+00
  end do
  !$omp end do
!
!  Average the boundary values, to come up with a reasonable
!  initial value for the interior.
!
  !$omp do reduction ( + : mean )
  do i = 2, m - 1
    mean = mean + w(i,1) + w(i,n)
  end do
  !$omp end do

  !$omp do reduction ( + : mean )
  do j = 1, n
    mean = mean + w(1,j) + w(m,j)
  end do
  !$omp end do

!$omp end parallel
!
!  OpenMP note:
!  You cannot normalize MEAN inside the parallel region.  It
!  only gets its correct value once you leave the parallel region.
!  So we interrupt the parallel region, set MEAN, and go back in.
!
  mean = mean / dble ( 2 * m + 2 * n - 4 )
  write ( *, '(a)' ) ' '
  write ( *, '(a,g14.6)' ) '  MEAN = ', mean
!
!  Initialize the interior solution to the mean value.
!
!$omp parallel shared ( mean, w ) private ( i, j )

  !$omp do
  do j = 2, n - 1
    do i = 2, m - 1
      w(i,j) = mean
    end do
  end do
  !$omp end do

!$omp end parallel
!
!  Iterate until the  new solution W differs from the old solution U
!  by no more than EPS.
!
  iterations = 0
  iterations_print = 1

  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) ' Iteration  Change'
  write ( *, '(a)' ) ' '

  wtime = omp_get_wtime ( )

  diff = eps

  do while ( eps <= diff )
!
!  OpenMP node: You CANNOT set DIFF to 0.0 inside the parallel region.
!
    diff = 0.0D+00

!$omp parallel shared ( u, w ) private ( i, j ) 

    !$omp do
    do j = 1, n
      do i = 1, m
        u(i,j) = w(i,j)
      end do
    end do
    !$omp end do

    !$omp do
    do j = 2, n - 1
      do i = 2, m - 1
        w(i,j) = 0.25D+00 * ( u(i-1,j) + u(i+1,j) + u(i,j-1) + u(i,j+1) )
      end do
    end do
    !$omp end do

    !$omp do reduction ( max : diff )
    do j = 1, n
      do i = 1, m
        diff = max ( diff, abs ( u(i,j) - w(i,j) ) )
      end do
    end do
    !$omp end do

!$omp end parallel

    iterations = iterations + 1

    if ( iterations == iterations_print ) then
      write ( *, '(2x,i8,2x,g14.6)' ) iterations, diff
      iterations_print = 2 * iterations_print
    end if

  end do

  wtime = omp_get_wtime ( ) - wtime

  write ( *, '(a)' ) ' '
  write ( *, '(2x,i8,2x,g14.6)' ) iterations, diff
  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) '  Error tolerance achieved.'
  write ( *, '(a,g14.6)' ) '  Wall clock time = ', wtime
!
!  Terminate.
!
  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) 'HEATED_PLATE_OPENMP:'
  write ( *, '(a)' ) '  Normal end of execution.'

  stop
end
                                                    make_gcc_exe.sh                                                                                     0000755 1322746 1322746 00000004006 13054132110 012665  0                                                                                                    ustar   ppk652                          ppk652                                                                                                                                                                                                                 #!/bin/bash
#
#  Compile the programs with GCC.
#
module load gcc
g++ -O3 -fopenmp -o fft_openmp_cpp fft_openmp.cpp -lm
gfortran -O3 -fopenmp -o heated_plate_f90 heated_plate_openmp.f90 -lm
gfortran -O3 -fopenmp -o  md_f90  md_openmp.f90 -lm
gcc -O3 -fopenmp -o pi_red pi_red.c 

echo 'pi calculation'
code=pi_red
export OMP_DYNAMIC=FALSE
export OMP_NUM_THREADS=1
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=2
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=4
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=8
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=16
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=20
time ./$code >& $code.$OMP_NUM_THREADS


echo 'DFT'
code=heated_plate_f90

export OMP_DYNAMIC=FALSE
export OMP_NUM_THREADS=1
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=2
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=4
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=8
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=16
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=20
time ./$code >& $code.$OMP_NUM_THREADS

echo 'MD'

code=md_f90

export OMP_DYNAMIC=FALSE
export OMP_NUM_THREADS=1
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=2
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=4
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=8
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=16
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=20
time ./$code >& $code.$OMP_NUM_THREADS

echo 'FFT'
code=fft_openmp_cpp

export OMP_DYNAMIC=FALSE
export OMP_NUM_THREADS=1
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=2
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=4
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=8
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=16
time ./$code >& $code.$OMP_NUM_THREADS
export OMP_NUM_THREADS=20
time ./$code >& $code.$OMP_NUM_THREADS


                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          md_openmp.f90                                                                                       0000644 1322746 1322746 00000031467 13054131625 012257  0                                                                                                    ustar   ppk652                          ppk652                                                                                                                                                                                                                 program main

!*****************************************************************************80
!
!! MAIN is the main program for MD_OPENMP.
!
!  Discussion:
!
!    MD_OPENMP implements a simple molecular dynamics simulation.
!
!    The program uses OpenMP directives to allow parallel computation.
!
!    The velocity Verlet time integration scheme is used. 
!
!    The particles interact with a central pair potential.
!
!  Licensing:
!
!    This code is distributed under the GNU LGPL license. 
!
!  Modified:
!
!    30 July 2009
!
!  Author:
!
!    Original FORTRAN90 version by Bill Magro.
!    This FORTRAN90 version by John Burkardt.
!
  use omp_lib

  implicit none

  integer ( kind = 4 ), parameter :: nd = 3
  integer ( kind = 4 ), parameter :: np = 1000

  real ( kind = 8 ) acc(nd,np)
  real ( kind = 8 ) box(nd)
  real ( kind = 8 ), parameter :: dt = 0.0001D+00
  real ( kind = 8 ) e0
  real ( kind = 8 ) force(nd,np)
  integer ( kind = 4 ) id
  real ( kind = 8 ) kinetic
  real ( kind = 8 ), parameter :: mass = 1.0D+00
  real ( kind = 8 ) pos(nd,np)
  real ( kind = 8 ) potential
  integer ( kind = 4 ) proc_num
  integer ( kind = 4 ) seed
  integer ( kind = 4 ) step
  integer ( kind = 4 ), parameter :: step_num = 400
  integer ( kind = 4 ) step_print
  integer ( kind = 4 ) step_print_index
  integer ( kind = 4 ) step_print_num
  integer ( kind = 4 ) thread_num
  real ( kind = 8 ) vel(nd,np)
  real ( kind = 8 ) wtime

  call timestamp ( )

  proc_num = omp_get_num_procs ( )
  thread_num = omp_get_max_threads ( )

  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) 'MD_OPENMP'
  write ( *, '(a)' ) '  FORTRAN90/OpenMP version'
  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) '  A molecular dynamics program.'
  write ( *, '(a)' ) ' '
  write ( *, '(a,i8)' ) &
    '  NP, the number of particles in the simulation is ', np
  write ( *, '(a,i8)' ) '  STEP_NUM, the number of time steps, is ', step_num
  write ( *, '(a,g14.6)' ) '  DT, the size of each time step, is ', dt
  write ( *, '(a)' ) ' '
  write ( *, '(a,i8)' ) '  The number of processors available is: ', proc_num
  write ( *, '(a,i8)' ) '  The number of threads available is:    ', thread_num
!
!  Set the dimensions of the box.
!
  box(1:nd) = 10.0D+00
!
!  Set initial positions, velocities, and accelerations.
!
  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) '  Initializing positions, velocities, and accelerations.'

  seed = 123456789
  call initialize ( np, nd, box, seed, pos, vel, acc )
!
!  Compute the forces and energies.
!
  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) '  Computing initial forces and energies.'

  call compute ( np, nd, pos, vel, mass, force, potential, kinetic )
!
!  Save the initial total energy for use in the accuracy check.
!
  e0 = potential + kinetic
!
!  This is the main time stepping loop:
!    Compute forces and energies,
!    Update positions, velocities, accelerations.
!
  step_print = 0
  step_print_index = 0
  step_print_num = 10
  
  step = 0
  write ( *, '(2x,i8,2x,g14.6,2x,g14.6,2x,g14.6)' ) &
    step, potential, kinetic, ( potential + kinetic - e0 ) / e0
  step_print_index = step_print_index + 1
  step_print = ( step_print_index * step_num ) / step_print_num

  wtime = omp_get_wtime ( )

  do step = 1, step_num

    call compute ( np, nd, pos, vel, mass, force, potential, kinetic )

    if ( step == step_print ) then

      write ( *, '(2x,i8,2x,g14.6,2x,g14.6,2x,g14.6)' ) &
        step, potential, kinetic, ( potential + kinetic - e0 ) / e0

      step_print_index = step_print_index + 1
      step_print = ( step_print_index * step_num ) / step_print_num

    end if

    call update ( np, nd, pos, vel, force, acc, mass, dt )

  end do

  wtime = omp_get_wtime ( ) - wtime
  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) '  Elapsed time for main computation:'
  write ( *, '(2x,g14.6,a)' ) wtime, ' seconds'
!
!  Terminate.
!
  write ( *, '(a)' ) ' '
  write ( *, '(a)' ) 'MD_OPENMP'
  write ( *, '(a)' ) '  Normal end of execution.'

  write ( *, '(a)' ) ' '
  call timestamp ( )

  stop
end
subroutine compute ( np, nd, pos, vel, mass, f, pot, kin )

!*****************************************************************************80
!
!! COMPUTE computes the forces and energies.
!
!  Discussion:
!
!    The computation of forces and energies is fully parallel.
!
!    The potential function V(X) is a harmonic well which smoothly
!    saturates to a maximum value at PI/2:
!
!      v(x) = ( sin ( min ( x, PI2 ) ) )^2
!
!    The derivative of the potential is:
!
!      dv(x) = 2.0D+00 * sin ( min ( x, PI2 ) ) * cos ( min ( x, PI2 ) )
!            = sin ( 2.0 * min ( x, PI2 ) )
!
!  Licensing:
!
!    This code is distributed under the GNU LGPL license. 
!
!  Modified:
!
!    15 July 2008
!
!  Author:
!
!    Original FORTRAN90 version by Bill Magro.
!    This FORTRAN90 version by John Burkardt.
!
!  Parameters:
!
!    Input, integer ( kind = 4 ) NP, the number of particles.
!
!    Input, integer ( kind = 4 ) ND, the number of spatial dimensions.
!
!    Input, real ( kind = 8 ) POS(ND,NP), the position of each particle.
!
!    Input, real ( kind = 8 ) VEL(ND,NP), the velocity of each particle.
!
!    Input, real ( kind = 8 ) MASS, the mass of each particle.
!
!    Output, real ( kind = 8 ) F(ND,NP), the forces.
!
!    Output, real ( kind = 8 ) POT, the total potential energy.
!
!    Output, real ( kind = 8 ) KIN, the total kinetic energy.
!
  implicit none

  integer ( kind = 4 ) np
  integer ( kind = 4 ) nd

  real ( kind = 8 ) d
  real ( kind = 8 ) d2
  real ( kind = 8 ) f(nd,np)
  integer ( kind = 4 ) i
  integer ( kind = 4 ) j
  real ( kind = 8 ) kin
  real ( kind = 8 ) mass
  real ( kind = 8 ), parameter :: PI2 = 3.141592653589793D+00 / 2.0D+00
  real ( kind = 8 ) pos(nd,np)
  real ( kind = 8 ) pot
  real ( kind = 8 ) rij(nd)
  real ( kind = 8 ) vel(nd,np)

  pot = 0.0D+00
  kin = 0.0D+00

!$omp parallel &
!$omp shared ( f, nd, np, pos, vel ) &
!$omp private ( d, d2, i, j, rij )

!$omp do reduction ( + : pot, kin )

  do i = 1, np
!
!  Compute the potential energy and forces.
!
    f(1:nd,i) = 0.0D+00

    do j = 1, np

      if ( i /= j ) then

        call dist ( nd, pos(1,i), pos(1,j), rij, d )
!
!  Attribute half of the potential energy to particle J.
!
        d2 = min ( d, PI2 )

        pot = pot + 0.5D+00 * ( sin ( d2 ) )**2

        f(1:nd,i) = f(1:nd,i) - rij(1:nd) * sin ( 2.0D+00 * d2 ) / d

      end if

    end do
!
!  Compute the kinetic energy.
!
    kin = kin + sum ( vel(1:nd,i)**2 )

  end do
!$omp end do

!$omp end parallel

  kin = kin * 0.5D+00 * mass
  
  return
end
subroutine dist ( nd, r1, r2, dr, d )

!*****************************************************************************80
!
!! DIST computes the displacement and distance between two particles.
!
!  Licensing:
!
!    This code is distributed under the GNU LGPL license. 
!
!  Modified:
!
!    17 March 2002
!
!  Author:
!
!    Original FORTRAN90 version by Bill Magro.
!    This FORTRAN90 version by John Burkardt.
!
!  Parameters:
!
!    Input, integer ( kind = 4 ) ND, the number of spatial dimensions.
!
!    Input, real ( kind = 8 ) R1(ND), R2(ND), the positions of the particles.
!
!    Output, real ( kind = 8 ) DR(ND), the displacement vector.
!
!    Output, real ( kind = 8 ) D, the Euclidean norm of the displacement,
!    in other words, the distance between the two particles.
!
  implicit none

  integer ( kind = 4 ) nd

  real ( kind = 8 ) d
  real ( kind = 8 ) dr(nd)
  real ( kind = 8 ) r1(nd)
  real ( kind = 8 ) r2(nd)

  dr(1:nd) = r1(1:nd) - r2(1:nd)

  d = sqrt ( sum ( dr(1:nd)**2 ) )

  return
end
subroutine initialize ( np, nd, box, seed, pos, vel, acc )

!*****************************************************************************80
!
!! INITIALIZE initializes the positions, velocities, and accelerations.
!
!  Licensing:
!
!    This code is distributed under the GNU LGPL license. 
!
!  Modified:
!
!    21 November 2007
!
!  Author:
!
!    Original FORTRAN90 version by Bill Magro.
!    This FORTRAN90 version by John Burkardt.
!
!  Parameters:
!
!    Input, integer ( kind = 4 ) NP, the number of particles.
!
!    Input, integer ( kind = 4 ) ND, the number of spatial dimensions.
!
!    Input, real ( kind = 8 ) BOX(ND), specifies the maximum position
!    of particles in each dimension.
!
!    Input/output, integer ( kind = 4 ) SEED, a seed for the random 
!    number generator.
!
!    Output, real ( kind = 8 ) POS(ND,NP), the position of each particle.
!
!    Output, real ( kind = 8 ) VEL(ND,NP), the velocity of each particle.
!
!    Output, real ( kind = 8 ) ACC(ND,NP), the acceleration of each particle.
!
  implicit none

  integer ( kind = 4 ) np
  integer ( kind = 4 ) nd

  real ( kind = 8 ) acc(nd,np)
  real ( kind = 8 ) box(nd)
  integer ( kind = 4 ) j
  integer ( kind = 4 ) seed
  real ( kind = 8 ) pos(nd,np)
  real ( kind = 8 ) r8_uniform_01
  real ( kind = 8 ) vel(nd,np)
!
!  Start by setting the positions to random numbers between 0 and 1.
!
  call random_number ( harvest = pos(1:nd,1:np) )
!
!  Use these random values as scale factors to pick random locations
!  inside the box.
!
!$omp parallel &
!$omp shared ( box, nd, np, pos ) &
!$omp private ( j )

!$omp do

  do j = 1, np
    pos(1:nd,j) = box(1:nd) * pos(1:nd,j)
  end do

!$omp end do
!$omp end parallel
!
!  Velocities and accelerations begin at 0.
!
!$omp parallel &
!$omp shared ( acc, nd, np, vel )

!$omp workshare

  vel(1:nd,1:np) = 0.0D+00
  acc(1:nd,1:np) = 0.0D+00

!$omp end workshare

!$omp end parallel

  return
end
subroutine timestamp ( )

!*****************************************************************************80
!
!! TIMESTAMP prints the current YMDHMS date as a time stamp.
!
!  Example:
!
!    31 May 2001   9:45:54.872 AM
!
!  Licensing:
!
!    This code is distributed under the GNU LGPL license.
!
!  Modified:
!
!    18 May 2013
!
!  Author:
!
!    John Burkardt
!
!  Parameters:
!
!    None
!
  implicit none

  character ( len = 8 ) ampm
  integer ( kind = 4 ) d
  integer ( kind = 4 ) h
  integer ( kind = 4 ) m
  integer ( kind = 4 ) mm
  character ( len = 9 ), parameter, dimension(12) :: month = (/ &
    'January  ', 'February ', 'March    ', 'April    ', &
    'May      ', 'June     ', 'July     ', 'August   ', &
    'September', 'October  ', 'November ', 'December ' /)
  integer ( kind = 4 ) n
  integer ( kind = 4 ) s
  integer ( kind = 4 ) values(8)
  integer ( kind = 4 ) y

  call date_and_time ( values = values )

  y = values(1)
  m = values(2)
  d = values(3)
  h = values(5)
  n = values(6)
  s = values(7)
  mm = values(8)

  if ( h < 12 ) then
    ampm = 'AM'
  else if ( h == 12 ) then
    if ( n == 0 .and. s == 0 ) then
      ampm = 'Noon'
    else
      ampm = 'PM'
    end if
  else
    h = h - 12
    if ( h < 12 ) then
      ampm = 'PM'
    else if ( h == 12 ) then
      if ( n == 0 .and. s == 0 ) then
        ampm = 'Midnight'
      else
        ampm = 'AM'
      end if
    end if
  end if

  write ( *, '(i2.2,1x,a,1x,i4,2x,i2,a1,i2.2,a1,i2.2,a1,i3.3,1x,a)' ) &
    d, trim ( month(m) ), y, h, ':', n, ':', s, '.', mm, trim ( ampm )

  return
end
subroutine update ( np, nd, pos, vel, f, acc, mass, dt )

!*****************************************************************************80
!
!! UPDATE updates positions, velocities and accelerations.
!
!  Discussion:
!
!    The time integration is fully parallel.
!
!    A velocity Verlet algorithm is used for the updating.
!
!    x(t+dt) = x(t) + v(t) * dt + 0.5 * a(t) * dt * dt
!    v(t+dt) = v(t) + 0.5 * ( a(t) + a(t+dt) ) * dt
!    a(t+dt) = f(t) / m
!
!  Licensing:
!
!    This code is distributed under the GNU LGPL license. 
!
!  Modified:
!
!    21 November 2007
!
!  Author:
!
!    Original FORTRAN90 version by Bill Magro.
!    This FORTRAN90 version by John Burkardt.
!
!  Parameters:
!
!    Input, integer ( kind = 4 ) NP, the number of particles.
!
!    Input, integer ( kind = 4 ) ND, the number of spatial dimensions.
!
!    Input/output, real ( kind = 8 ) POS(ND,NP), the position of each particle.
!
!    Input/output, real ( kind = 8 ) VEL(ND,NP), the velocity of each particle.
!
!    Input, real ( kind = 8 ) F(ND,NP), the force on each particle.
!
!    Input/output, real ( kind = 8 ) ACC(ND,NP), the acceleration of each
!    particle.
!
!    Input, real ( kind = 8 ) MASS, the mass of each particle.
!
!    Input, real ( kind = 8 ) DT, the time step.
!
  implicit none

  integer ( kind = 4 ) np
  integer ( kind = 4 ) nd

  real ( kind = 8 ) acc(nd,np)
  real ( kind = 8 ) dt
  real ( kind = 8 ) f(nd,np)
  integer ( kind = 4 ) i
  integer ( kind = 4 ) j
  real ( kind = 8 ) mass
  real ( kind = 8 ) pos(nd,np)
  real ( kind = 8 ) rmass
  real ( kind = 8 ) vel(nd,np)

  rmass = 1.0D+00 / mass

!$omp parallel &
!$omp shared ( acc, dt, f, nd, np, pos, rmass, vel ) &
!$omp private ( i, j )

!$omp do
  do j = 1, np
    do i = 1, nd
      pos(i,j) = pos(i,j) + vel(i,j) * dt + 0.5D+00 * acc(i,j) * dt * dt
      vel(i,j) = vel(i,j) + 0.5D+00 * dt * ( f(i,j) * rmass + acc(i,j) )
      acc(i,j) = f(i,j) * rmass
    end do
  end do
!$omp end do

!$omp end parallel

  return
end
                                                                                                                                                                                                         pi_red                                                                                              0000775 1322746 1322746 00000015446 13054132071 011146  0                                                                                                    ustar   ppk652                          ppk652                                                                                                                                                                                                                 ELF          >    @     @                 @ 8  @         @       @ @     @ @     �      �                           @      @                                          @       @     �      �                    �      �`     �`           @                    �      �`     �`     �      �                         @     @     D       D              P�td         @     @     $       $              Q�td                                                  /lib64/ld-linux-x86-64.so.2          GNU                       GNU 돱��Bh�F�S�$|J)�                                                     8                                                                    ?                       libm.so.6 __gmon_start__ _Jv_RegisterClasses libc.so.6 printf __libc_start_main GLIBC_2.2.5            .          ui	   Q       p	`                   �	`                   �	`                   H���k   ��   �  H����5�  �%�  @ �%�  h    ������%�  h   �����1�I��^H��H���PTI���@ H���@ H�� @ ��������H��H�)  H��t��H��Ð������������UH��SH���=@   uK��` H�:  H��` H��H��H9�s$fD  H��H�  �Ÿ` H�  H9�r���  H��[��fff.�     H�=�   UH��t�    H��t��` ����Ð�������������H��H��  1��%�  H�D$H��  H��  �H*�f(�H��H��      �\$�^�f(���  ~^�5�  1��-�  �    �H*�H��f(�H9��X��Y�f(��Y��X��^��X�u��  H�#  �  �Yÿ�@ �   �  � ���1�H��Ð����������fffff.�     H�l$�L�d$�H�-�  L�%�  L�l$�L�t$�L�|$�H�\$�H��8L)�A��I��H��I������H��t1�@ L��L��D��A��H��H9�r�H�\$H�l$L�d$L�l$ L�t$(L�|$0H��8Ð������UH��SH��H�  H���t��` D  H����H�H���u�H��[�Ð�H������H���                Computed PI %.24f
            �?      �?      @;$      ����@   ����X   ����p              zR x�        �����    D �   4   x���           $   L   p����    Q��_@����X           ��������        ��������                                     .              �@            �@     ���o    `@            �@            �@     
       ]                                           x	`            0                            �@            �@                   	              ���o    `@     ���o           ���o    V@                                                                                                             �`                     �@     @              �*   GCC: (GNU) 4.4.7 20120313 (Red Hat 4.4.7-16) GCC: (GNU) 4.4.7 20120313 (Red Hat 4.4.7-3)  .symtab .strtab .shstrtab .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt .init .text .fini .rodata .eh_frame_hdr .eh_frame .ctors .dtors .jcr .dynamic .got .got.plt .data .bss .comment                                                                                @                                          #             @                                          1             <@     <      $                              D   ���o       `@     `                                   N             �@     �      x                           V             �@     �      ]                              ^   ���o       V@     V      
                            k   ���o       `@     `                                   z             �@     �                                  �             �@     �      0                           �             �@     �                                    �             �@     �      0                             �             @           �                             �             �@     �                                    �             �@     �      @                              �             @           $                              �             0@     0      t                              �             �`     �                                    �             �`     �                                    �             �`     �                                    �             �`     �      �                           �             p	`     p	                                   �             x	`     x	      (                             �             �	`     �	                                    �             �	`     �	      8                              �      0               �	      Y                                                   	
      �                                                    �      �         .                 	                                                                                        @                   @                   <@                   `@                   �@                   �@                   V@                   `@                  	 �@                  
 �@                   �@                   �@                   @                   �@                   �@                   @                   0@                   �`                   �`                   �`                   �`                   p	`                   x	`                   �	`                   �	`                                        <@                 ��                     �`             *     �`             8     �`             E     `@             [     �	`            j     �	`            x     �@                 ��                �     �`             �     �@             �     �`             �     �@             �    ��                �     x	`             �      �`             �      �`                 �`                  �	`                                  ,    �@            <    @             C                      R                      f    �@             l                     �    �	`            �    �	`            �    �@            �    �	`             �    �	`            �   �@             �   �`             �    �@     �       �    �	`            �   ���	`             �    �	`            �   ���	`             �   ���	`             �    �	`                 @     �           �@              call_gmon_start crtstuff.c __CTOR_LIST__ __DTOR_LIST__ __JCR_LIST__ __do_global_dtors_aux completed.6349 dtor_idx.6351 frame_dummy __CTOR_END__ __FRAME_END__ __JCR_END__ __do_global_ctors_aux pi_red.c _GLOBAL_OFFSET_TABLE_ __init_array_end __init_array_start _DYNAMIC data_start printf@@GLIBC_2.2.5 __libc_csu_fini _start __gmon_start__ _Jv_RegisterClasses _fini __libc_start_main@@GLIBC_2.2.5 x sum _IO_stdin_used __data_start i __dso_handle __DTOR_END__ __libc_csu_init num_steps __bss_start pi _end _edata step main _init                                                                                                                                                                                                                           pi_red.c                                                                                            0000664 1322746 1322746 00000000733 13054131646 011364  0                                                                                                    ustar   ppk652                          ppk652                                                                                                                                                                                                                 #include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#define NSTEPS  5000000000 

long i,num_steps=NSTEPS;
double x,step,sum,pi;


int main(int argc, char **argv)
{
x=0;
sum = 0.0;
step = 1.0/(double) num_steps;
#pragma omp parallel private(i,x) shared(sum)
{
#pragma omp for schedule(static) reduction(+:sum)
for (i=0; i < num_steps; ++i) {
       x = (i+0.5)*step;
       sum = sum+ 4.0/(1.0+x*x);
}
}
    pi = step * sum;
printf("Computed PI %.24f\n", pi);
return 0;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     