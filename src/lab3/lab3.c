#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define G 6.67e-11
#define M 10000

int N;
long double delta_t = 1;

void computeForce(long double *x, long double *y, int bid, long double *fx, long double *fy)
{
    int i, x0 = x[bid], y0 = y[bid];
    long double r, sumx = 0, sumy = 0, cosine, sine;
    for (i = 0; i < N; i++)
    {
        if (i == bid)
        {
            continue;
        }
        r = sqrtl((x[i] - x0) * (x[i] - x0) + (y[i] - y0) * (y[i] - y0));
        cosine = (x[i] - x0) / r;
        sine = (y[i] - y0) / r;
        if (r < 2)
        {
            r = 2;
        }
        sumx += 100 * 100 * G * M * M * cosine / (r * r);
        sumy += 100 * 100 * G * M * M * sine / (r * r);
    }
    fx[bid] = sumx;
    fy[bid] = sumy;
}

void computeVelocities(long double *vx, long double *vy, long double *fx, long double *fy, int bid)
{
    vx[bid] += fx[bid] * delta_t / M;
    vy[bid] += fy[bid] * delta_t / M;
}

void computePositions(long double *x, long double *y, long double *vx, long double *vy, int bid)
{
    x[bid] += vx[bid] * delta_t / 100;
    y[bid] += vy[bid] * delta_t / 100;
}

int main(int argc, char *argv[])
{
    int size, rank, tag = 1;
    int len;
    int i, j;
    long double *x, *y, *fx, *fy, *vx, *vy;
    double start, end;
    int steps = atoi(argv[2]);
    N = atoi(argv[1]);
    len = sqrt(N);
    x = (long double *)malloc(N * sizeof(long double));
    y = (long double *)malloc(N * sizeof(long double));
    fx = (long double *)malloc(N * sizeof(long double));
    fy = (long double *)malloc(N * sizeof(long double));
    vx = (long double *)malloc(N * sizeof(long double));
    vy = (long double *)malloc(N * sizeof(long double));
    for (i = 0; i < N; i++)
    {
        vx[i] = 0;
        vy[i] = 0;
        x[i] = i % len;
        y[i] = i / len;
    }
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (0 == rank)
    {
        start = MPI_Wtime();
    }
    for (i = 0; i < steps; i++)
    {
        for (j = N / size * rank; j < N / size * rank + N / size; j++)
        {
            computeForce(x, y, j, fx, fy);
            computeVelocities(vx, vy, fx, fy, j);
            computePositions(x, y, vx, vy, j);
        }
        if (size > 1)
        {
            for (j = 0; j < size; j++)
            {
                MPI_Bcast(&x[N / size * j], N / size, MPI_LONG_DOUBLE, j, MPI_COMM_WORLD);
                MPI_Bcast(&y[N / size * j], N / size, MPI_LONG_DOUBLE, j, MPI_COMM_WORLD);
                MPI_Bcast(&vx[N / size * j], N / size, MPI_LONG_DOUBLE, j, MPI_COMM_WORLD);
                MPI_Bcast(&vy[N / size * j], N / size, MPI_LONG_DOUBLE, j, MPI_COMM_WORLD);
                MPI_Bcast(&fx[N / size * j], N / size, MPI_LONG_DOUBLE, j, MPI_COMM_WORLD);
                MPI_Bcast(&fy[N / size * j], N / size, MPI_LONG_DOUBLE, j, MPI_COMM_WORLD);
            }
        }
    }
    if (0 == rank)
    {
        end = MPI_Wtime();
        printf(" %3d | %.3lf\n", N, end - start);
    }
    MPI_Finalize();
    return 0;
}
