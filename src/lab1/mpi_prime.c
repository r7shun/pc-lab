#include <stdio.h>
#include "mpi.h"
#include <time.h>
#include <math.h>

const int num[] = {1000, 10000, 100000, 500000};

int isPrime(int n)
{
    int sq = sqrt((double)n);
    for (int i = 2; i <= sq; i++)
    {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    double start, end, total_time;
    int size, myid, i, n, count, sum;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    for (i = 0; i < 4; i++)
    {
        count = 0;
        sum = 0;
        n = num[i];
        total_time = 0.0;
        if (myid == 0)
            start = MPI_Wtime();
        for (int j = myid; j <= n; j += size)
            count += isPrime(j);
        MPI_Reduce(&count, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (myid == 0)
        {
            end = MPI_Wtime();
            total_time += end - start;
        }
        if (myid == 0)
            printf(" %6d | %.3lf | %d\n", n, (total_time)*1000, sum-2);
    }
    MPI_Finalize();
    return 0;
}
