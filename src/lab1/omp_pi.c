#include <stdio.h>
#include <time.h>
#include <omp.h>
//由于求pi程序运行时间短, 求1000次pi值的平均值保证准确率
#define C 1000
const int num[] = {1000,10000,100000,500000};

int main(void) {
    clock_t start, end, total_time;
    double x, step, pi, sum;
    int i,n;
    int thread_n;
    printf("t |    n   | time(ms) |      pi      \n");
    for (thread_n=1; thread_n <= 8; thread_n *= 2) {
        omp_set_num_threads(thread_n);
        for(i = 0; i < 4; i++) {
            n = num[i];
            step = 1.0 / n;
            total_time = 0;
            for(int k = 0; k < C; k++) {
                start = clock();
                sum = 0.0;
                #pragma omp parallel for reduction(+:sum)
                for(int j = 0; j < n; j++) {
                    x = step * (j + 0.5);
                    sum += 4.0 / (1.0 + x * x);
                }
                pi = step * sum;
                end = clock();
                total_time += end - start;
            }
            printf("%d | %6d | %lf | %.10lf\n", thread_n, n, (double)total_time/CLOCKS_PER_SEC*1000/C,pi);
        }
    }
    return 0;
}
