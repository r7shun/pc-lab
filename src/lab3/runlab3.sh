mpicc lab3.c -o lab3 -lm
echo -e "\nthread_n = 1"
mpiexec  -n 1 ./lab3 64 1000
mpiexec  -n 1 ./lab3 256 1000
echo -e "\nthread_n = 2"
mpiexec  -n 2 ./lab3 64 1000
mpiexec  -n 2 ./lab3 256 1000
#echo -e "\nthread_n = 4"
#mpirun  -np 4 ./lab3 64 1000
#mpirun  -np 4 ./lab3 256 1000
#echo -e "\nthread_n = 8"
#mpirun  -np 8 ./lab3 64 1000
#mpirun  -np 8 ./lab3 256 1000
