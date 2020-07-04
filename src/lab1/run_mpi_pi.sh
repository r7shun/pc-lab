mpicc mpi_pi.c -o mpi_pi
echo -e "\nthread_n = 1"
echo "    n   | t(ms) |    pi    "
mpiexec -n 1 ./mpi_pi
echo -e "\nthread_n = 2"
echo "    n   | t(ms) |    pi    "
mpiexec -n 2 ./mpi_pi
echo -e "\nthread_n = 4"
echo "    n   | t(ms) |    pi    "
mpiexec -n 4 ./mpi_pi
echo -e "\nthread_n = 8"
echo "    n   | t(ms) |    pi    "
mpiexec -n 8 ./mpi_pi
