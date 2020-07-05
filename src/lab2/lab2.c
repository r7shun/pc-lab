#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#define BUCKET 80000
#define V_MAX 40
#define P 0.3

typedef struct car
{
    int velocity;
    struct car *next;
} Car;

typedef struct
{
    Car *head;
    int count;
} CarList;

const int car_num[] = {100000, 500000, 1000000};
const int cycles[] = {2000, 500, 300};
CarList road[BUCKET];
int have_car[BUCKET];
int temp[BUCKET];

void calcNextV(int myid)
{
    // 由于使用srand函数生成随机数, 所以并行时各进程产生的随机结果大致相同, 导致最终结果相当于车辆数目只有规模/进程数的模拟情况, 再每个位置乘以进程数, 所以进程数不同时, 结果完全不相似. 为了避免上述情况srand后增加迭代次数为myid*500的循环, 这样每个进程结果大致相同
    srand((unsigned int)time(NULL));
    for (int i = 0; i < myid * 500; i++)
        rand();
    int j;
    for (j = BUCKET - 1; j >= 0; j--)
        if (temp[j] > 0)
            break;
    for (int i = j; i >= 0; i--)
    {
        if (road[i].count == 0)
        {
            if (temp[i] > 0)
                j = i;
            continue;
        }
        Car *head = road[i].head;
        Car *p;
        if (i == j)
        {
            //no cars in the front.
            for (p = head; p != NULL; p = p->next)
            {
                if (p->velocity < V_MAX)
                    p->velocity++;
                if (rand() <= P * RAND_MAX)
                    p->velocity--;
            }
        }
        else
        {
            int distance = j - i;
            for (p = head; p != NULL; p = p->next)
            {
                if (distance <= p->velocity)
                    p->velocity = (distance - 1 < 0) ? 0 : (distance - 1);
                else if (p->velocity < V_MAX)
                    p->velocity++;
                if (p->velocity > 0 && rand() <= P * RAND_MAX)
                    p->velocity--;
            }
        }
        j = i;
    }
}

void moveCars()
{
    for (int i = BUCKET - 1; i >= 0; i--)
    {
        have_car[i] = 0;
        if (road[i].count == 0)
            continue;
        Car *p = road[i].head;
        Car *q = p;
        while (p != NULL)
        {
            if (p->velocity == 0)
            {
                //not moving
                q = p;
                p = p->next;
                have_car[i]++;
            }
            else
            {
                road[i].count--;
                if (p == road[i].head)
                    road[i].head = p->next;
                else
                    q->next = p->next;
                //insert p into target list.
                int new_location = i + p->velocity;
                if (new_location >= BUCKET)
                {
                    printf("out of range!\n");
                    exit(1);
                }
                else
                {
                    Car *next_p = p->next;
                    if (road[new_location].head != NULL)
                    {
                        Car *temp = road[new_location].head->next;
                        road[new_location].head->next = p;
                        p->next = temp;
                    }
                    else
                    {
                        road[new_location].head = p;
                        p->next = NULL;
                    }
                    p = next_p;
                    road[new_location].count++;
                    have_car[new_location]++;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    double st, ed;
    int size, myid;
    FILE *out_file;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    for (int i = 0; i < 3; i++)
    {
        //free loc
        for (int loc = 0; loc < BUCKET; loc++)
        {
            if (road[loc].count > 0)
            {
                road[loc].count = 0;
                Car *head = road[loc].head;
                while (head != NULL)
                {
                    Car *temp = head->next;
                    free(head);
                    head = temp;
                }
            }
            have_car[loc] = 0;
            road[loc].head = NULL;
        }
        have_car[0] = car_num[i];
        int CarNum = car_num[i] / size;
        road[0].count = CarNum;
        Car *tail;
        for (int j = 0; j < CarNum; j++)
        {
            Car *temp = (Car *)malloc(sizeof(Car));
            temp->next = NULL;
            temp->velocity = 0;
            if (j == 0)
            {
                tail = temp;
                road[0].head = temp;
            }
            else
            {
                tail->next = temp;
                tail = temp;
            }
        }

        if (myid == 0)
        {
            st = MPI_Wtime();
            printf("%7d | %4d | ", car_num[i], cycles[i]);
        }
        for (int j = 0; j < cycles[i]; j++)
        {
            calcNextV(myid);
            moveCars();
            MPI_Reduce(have_car, temp, BUCKET, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Bcast(temp, BUCKET, MPI_INT, 0, MPI_COMM_WORLD);
            int count = 0, count2 = 0;
            for (int loc = 0; loc < BUCKET; loc++)
            {
                count += temp[loc];
                count2 += have_car[loc];
            }
        }
        if (myid == 0)
        {
            ed = MPI_Wtime();
            printf("%.3lf\n", (double)(ed - st));
            char filename[] = "xxx.txt";
            sprintf(filename, "%d-%d.txt", i, size);
            out_file = fopen(filename, "w");
            if (out_file == NULL)
            {
                printf("open file error\n");
                exit(2);
            }
            int count = 0;
            for (int loc = 0; loc < BUCKET; loc++)
            {
                if (temp[loc] > 0)
                {
                    fprintf(out_file, "%d %d\n", loc, temp[loc]);
                    count += temp[loc];
                }
            }
            fclose(out_file);
        }
    }
    return 0;
}
