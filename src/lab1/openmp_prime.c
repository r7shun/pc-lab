#include <stdio.h>
#include <time.h>
#include <math.h>
#include <omp.h>

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

int main(void)
{
    printf("t |    n   |  t(ms)  |      pi      \n");
    int i, n, count;
    clock_t start, end, total_time;
    int thread_n;
    for (thread_n = 1; thread_n <= 8; thread_n *= 2)
    {

        for (i = 0; i < 4; i++)
        {
            n = num[i];
            count = 0;
            total_time = 0;
            start = clock();
            omp_set_num_threads(thread_n);
#pragma omp parallel for reduction(+ \
                                   : count)
            for (int j = 2; j <= n; j++)
                count += isPrime(j);
            end = clock();
            total_time += end - start;
            printf("%d | %6d | %.3lf | %d\n", thread_n, n, (double)(total_time) / CLOCKS_PER_SEC * 1000, count);
        }
    }
    return 0;
}

