#!/bin/bash
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


