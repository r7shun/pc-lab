#include <stdio.h>
#include "mpi.h"
#include <time.h>
#define C 1000
const int num[] = {1000, 10000, 50000, 100000};

int main(int argc, char **argv)
{
    double pi, step, sum, x, start, end, total_time;
    int size, myid;
    int i;
    int n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    for (i = 0; i < 4; i++)
    {
        n = num[i];
        total_time = 0.0;
        for (int times = 0; times < C; times++)
        {
            if (myid == 0)
                start = MPI_Wtime();
            step = 1.0 / n;
            sum = 0.0;
            int j;
            for (j = myid; j < n; j += size)
            {
                x = step * (j + 0.5);
                sum += 4.0 / (1 + x * x);
            }
            sum *= step;
            MPI_Reduce(&sum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            if (myid == 0)
            {
                end = MPI_Wtime();
                total_time += end - start;
            }
        }
        if (myid == 0)
            printf(" %6d | %.3lf | %.10lf\n", n, (double)total_time * 1000 / C, pi);
    }
    MPI_Finalize();
    return 0;
}
