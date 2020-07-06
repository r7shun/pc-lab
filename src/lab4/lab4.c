#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h"
#define INF 2147483647

// 合并两个已排好序的子数组A[l : m], A[m + 1 : r], 写回A[l : r]
void Merge(int *A, int l, int m, int r)
{
    int i, j, k, n1 = m - l + 1, n2 = r - m;
    int *L = (int *)malloc((n1 + 1) * sizeof(int));
    int *R = (int *)malloc((n2 + 1) * sizeof(int));
    for (i = 0; i < n1; i++)
        L[i] = A[l + i];
    for (j = 0; j < n2; j++)
        R[j] = A[m + 1 + j];
    L[i] = R[j] = INF;
    i = j = 0;
    for (k = l; k <= r; k++)
        if (L[i] <= R[j])
            A[k] = L[i++];
        else
            A[k] = R[j++];
    free(L);
    free(R);
}

// 对A[l : r]进行归并排序
void MergeSort(int *A, int l, int r)
{
    if (l < r)
    {
        int m = (l + r) / 2;
        MergeSort(A, l, m);
        MergeSort(A, m + 1, r);
        Merge(A, l, m, r);
    }
}

// 对A[0 : n - 1]进行PSRS排序
void PSRS(int *A, int n, int id, int num_processes)
{
    int per;
    int *samples, *global_samples;
    int *pivots, *sizes, *newsizes;
    int *offsets, *newoffsets;
    int *newdatas, newdatassize;
    int *global_sizes, *global_offsets;

    per = n / num_processes;
    samples = (int *)malloc(num_processes * sizeof(int));
    pivots = (int *)malloc(num_processes * sizeof(int));
    if (id == 0)
    {
        global_samples = (int *)malloc(num_processes * num_processes * sizeof(int));
    }
    // 设置路障，同步所有进程
    MPI_Barrier(MPI_COMM_WORLD);
    // 均匀划分
    MergeSort(A, id * per, (id + 1) * per - 1);
    // 正则采样，当前进程选出 num_processes 个样本放在local_sample中
    for (int k = 0; k < num_processes; k++)
        samples[k] = A[id * per + k * per / num_processes];
    // 主进程的sample收集各进程的local_sample，共 num_processes * num_processes 个
    MPI_Gather(samples, num_processes, MPI_INT, global_samples, num_processes, MPI_INT, 0, MPI_COMM_WORLD);

    // 采样排序 选择主元
    if (id == 0)
    {
        MergeSort(global_samples, 0, num_processes * num_processes - 1);
        for (int k = 0; k < num_processes - 1; k++)
            pivots[k] = global_samples[(k + 1) * num_processes];
        pivots[num_processes - 1] = INF; //哨兵
    }
    //主进程向各个进程广播，所有进程拥有一样的pivots数组
    MPI_Bcast(pivots, num_processes, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    sizes = (int *)calloc(num_processes, sizeof(int));
    offsets = (int *)calloc(num_processes, sizeof(int));
    newsizes = (int *)calloc(num_processes, sizeof(int));
    newoffsets = (int *)calloc(num_processes, sizeof(int));
    // 主元划分
    for (int k = 0, j = id * per; j < id * per + per; j++)
    {
        if (A[j] < pivots[k])
            sizes[k]++;
        else
            sizes[++k]++;
    }
    // 全局交换
    // 多对多全局交换消息，首先每个进程向每个接收者发送接收者对应的[段的大小]
    MPI_Alltoall(sizes, 1, MPI_INT, newsizes, 1, MPI_INT, MPI_COMM_WORLD);
    // 计算原来的段偏移数组，新的段偏移数组，新的数据大小
    newdatassize = newsizes[0];
    for (int k = 1; k < num_processes; k++)
    {
        offsets[k] = offsets[k - 1] + sizes[k - 1];
        newoffsets[k] = newoffsets[k - 1] + newsizes[k - 1];
        newdatassize += newsizes[k];
    }
    // 申请当前进程新的数据空间
    newdatas = (int *)malloc(newdatassize * sizeof(int));
    // 多对多全局交换消息，每个进程向每个接收者发送接收者对应的【段】
    MPI_Alltoallv(&(A[id * per]), sizes, offsets, MPI_INT, newdatas, newsizes, newoffsets, MPI_INT, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    // 当前进程归并排序自己的新数据
    MergeSort(newdatas, 0, newdatassize - 1);
    MPI_Barrier(MPI_COMM_WORLD);
    // 主进程收集各个进程的数据，写回A
    // 首先收集各进程新数据的大小
    if (id == 0)
        global_sizes = (int *)calloc(num_processes, sizeof(int));
    MPI_Gather(&newdatassize, 1, MPI_INT, global_sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // 主进程计算即将搜集的各进程数据的起始位置
    if (id == 0)
    {
        global_offsets = (int *)calloc(num_processes, sizeof(int));
        for (int k = 1; k < num_processes; k++)
            global_offsets[k] = global_offsets[k - 1] + global_sizes[k - 1];
    }
    // 主进程收集各个进程的数据
    MPI_Gatherv(newdatas, newdatassize, MPI_INT, A, global_sizes, global_offsets, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    // 销毁动态数组
    free(samples);
    samples = NULL;
    free(pivots);
    pivots = NULL;
    free(sizes);
    sizes = NULL;
    free(offsets);
    offsets = NULL;
    free(newdatas);
    newdatas = NULL;
    free(newsizes);
    newsizes = NULL;
    free(newoffsets);
    newoffsets = NULL;
    if (id == 0)
    {
        free(global_samples);
        global_samples = NULL;
        free(global_sizes);
        global_sizes = NULL;
        free(global_offsets);
        global_offsets = NULL;
    }
}

void init(int N, int *A)
{
    srand((unsigned int)time(NULL));
    for (int i = 0; i < N; i++)
        A[i] = rand() % (N + N);
}

int main(int argc, char *argv[])
{
    int N = atoi(argv[1]);
    int *A;
    A = (int *)malloc(N * sizeof(int));
    init(N, A);
    double t1, t2;
    int id, num_processes;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes); //获取进程数
    MPI_Comm_rank(MPI_COMM_WORLD, &id);            //获取当前进程id
    if (id == 0)
        t1 = MPI_Wtime();
    PSRS(A, N, id, num_processes);
    if (id == 0)
    {
        t2 = MPI_Wtime();
        printf(" %7d | ", N);
        printf(" %.3lf\n", (t2 - t1) * 1000);
        // 8线程N=64时会输出排序结果
        if (N == 64 && num_processes == 8)
        {
            printf("------------------------------------------------\n");
            for (int i = 0; i < N; i++)
            {
                printf("%3d ", A[i]);
                if ((i + 1) % 16 == 0)
                    printf("\n");
            }
        }
    }
    MPI_Finalize();
    return 0;
}