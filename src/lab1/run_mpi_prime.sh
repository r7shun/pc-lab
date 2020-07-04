mpicc mpi_prime.c -lm  -o mpi_prime
echo -e "\nthread_n = 1"
echo "    n   | t(ms) |   cnt   "
mpiexec -n 1 ./mpi_prime
echo -e "\nthread_n = 2"
echo "    n   | t(ms) |   cnt   "
mpiexec -n 2 ./mpi_prime
echo -e "\nthread_n = 4"
echo "    n   | t(ms) |   cnt   "
mpiexec -n 4 ./mpi_prime
echo -e "\nthread_n = 8"
echo "    n   | t(ms) |   cnt   "
mpiexec -n 8 ./mpi_prime

