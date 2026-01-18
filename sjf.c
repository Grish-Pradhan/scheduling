#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

/* ---------------- High-resolution timing ---------------- */
double now_ms() {
#ifdef _WIN32
    static LARGE_INTEGER freq;
    static int initialized = 0;
    LARGE_INTEGER counter;

    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
#endif
}

/* ---------------- Process structure ---------------- */
typedef struct {
    int pid;
    int arrival;
    int burst;
    int start;
    int completion;
    int waiting;
    int turnaround;
    int response;
    int done;
} Process;

/* ---------------- Main SJF Scheduling ---------------- */
int main() {
    int n;
    printf("Enter number of processes: ");
    scanf("%d", &n);

    double preprocess_start = now_ms();

    Process *p = malloc(sizeof(Process) * n);
    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        printf("P%d Arrival Time: ", p[i].pid);
        scanf("%d", &p[i].arrival);
        printf("P%d Burst Time  : ", p[i].pid);
        scanf("%d", &p[i].burst);
        p[i].done = 0;
    }

    double preprocess_end = now_ms();
    double schedule_start = now_ms();

    int completed = 0;
    int current_time = 0;
    int total_burst = 0;

    while (completed < n) {
        int idx = -1;
        int min_burst = 1e9;
        for (int i = 0; i < n; i++) {
            if (!p[i].done && p[i].arrival <= current_time) {
                if (p[i].burst < min_burst || 
                   (p[i].burst == min_burst && p[i].arrival < p[idx].arrival)) {
                    min_burst = p[i].burst;
                    idx = i;
                }
            }
        }

        if (idx == -1) { // CPU idle
            current_time++;
            continue;
        }

        p[idx].start = current_time;
        p[idx].completion = p[idx].start + p[idx].burst;
        p[idx].turnaround = p[idx].completion - p[idx].arrival;
        p[idx].waiting = p[idx].turnaround - p[idx].burst;
        p[idx].response = p[idx].start - p[idx].arrival;

        current_time = p[idx].completion;
        total_burst += p[idx].burst;
        p[idx].done = 1;
        completed++;
    }

    double schedule_end = now_ms();

    /* ---------------- Metrics ---------------- */
    double sum_wt = 0, sum_tat = 0, sum_rt = 0;
    int min_wt = p[0].waiting, max_wt = p[0].waiting;
    int min_tat = p[0].turnaround, max_tat = p[0].turnaround;
    int min_rt = p[0].response, max_rt = p[0].response;

    for (int i = 0; i < n; i++) {
        sum_wt += p[i].waiting;
        sum_tat += p[i].turnaround;
        sum_rt += p[i].response;

        if (p[i].waiting < min_wt) min_wt = p[i].waiting;
        if (p[i].waiting > max_wt) max_wt = p[i].waiting;

        if (p[i].turnaround < min_tat) min_tat = p[i].turnaround;
        if (p[i].turnaround > max_tat) max_tat = p[i].turnaround;

        if (p[i].response < min_rt) min_rt = p[i].response;
        if (p[i].response > max_rt) max_rt = p[i].response;
    }

    double avg_wt = sum_wt / n;
    double avg_tat = sum_tat / n;
    double avg_rt = sum_rt / n;

    double total_time = 0;
    int first_arrival = p[0].arrival;
    int last_completion = p[0].completion;
    for (int i = 0; i < n; i++) {
        if (p[i].arrival < first_arrival) first_arrival = p[i].arrival;
        if (p[i].completion > last_completion) last_completion = p[i].completion;
    }
    total_time = last_completion - first_arrival;

    double cpu_util = ((double)total_burst / total_time) * 100.0;
    double throughput = (double)n / total_time;

    /* ---------------- Output ---------------- */
    printf("\nPID\tAT\tBT\tST\tCT\tWT\tTAT\tRT\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               p[i].pid, p[i].arrival, p[i].burst, p[i].start,
               p[i].completion, p[i].waiting, p[i].turnaround,
               p[i].response);
    }

    printf("\n--- Statistics ---\n");
    printf("WT  : min=%d max=%d avg=%.2f\n", min_wt, max_wt, avg_wt);
    printf("TAT : min=%d max=%d avg=%.2f\n", min_tat, max_tat, avg_tat);
    printf("RT  : min=%d max=%d avg=%.2f\n", min_rt, max_rt, avg_rt);

    printf("\nCPU Utilization    : %.2f %%\n", cpu_util);
    printf("Throughput         : %.6f processes/ms\n", throughput);

    printf("\n--- Timing ---\n");
    printf("Pre-process time   : %.3f ms\n", preprocess_end - preprocess_start);
    printf("Scheduling time    : %.3f ms\n", schedule_end - schedule_start);
    printf("Total wall time    : %.3f ms\n", schedule_end - preprocess_start);

    free(p);
    return 0;
}
