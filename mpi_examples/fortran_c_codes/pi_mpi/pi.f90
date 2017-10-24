PROGRAM compute_pi
implicit none
integer i,n
double precision pi,h,x

n=900000000
pi = 0.0D+0
h = 1.0D+0/REAL(n)   !trapezoid base

do i=1,n
    x = h*(REAL(i)-0.5D+0)
    pi = pi+ (4.0D+0/(1.0D+0 + x*x))
end do

pi=pi*h
write(*,*) pi

END PROGRAM compute_pi
