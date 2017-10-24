#include <stdlib.h>
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
