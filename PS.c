#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#define MAX 20
#define PAGE_SIZE 4  // 4 KB pages
#define MEMORY_FRAMES 10  // Limited memory frames

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
    int priority;
    int start;
    int completion;
    int waiting;
    int turnaround;
    int response;
    int done;
    int memory_pages;
    int in_memory;
    int swap_in_count;
    int swap_out_count;
} Process;

void hardcoded_data(Process p[], int *n) {
    *n = 10;
    
    // Hardcoded test data from the image
    int arrival[] = {0, 1, 2, 4, 3, 2, 6, 5, 7, 8};
    int burst[] = {5, 3, 8, 5, 6, 2, 3, 4, 9, 7};
    int priority[] = {1, 3, 5, 6, 8, 9, 7, 5, 6, 3};
    int pages[] = {3, 2, 4, 2, 3, 1, 2, 3, 5, 4};  // Memory pages
    
    for (int i = 0; i < *n; i++) {
        p[i].pid = i + 1;
        p[i].arrival = arrival[i];
        p[i].burst = burst[i];
        p[i].priority = priority[i];
        p[i].memory_pages = pages[i];
        p[i].done = 0;
        p[i].in_memory = 0;
        p[i].swap_in_count = 0;
        p[i].swap_out_count = 0;
    }
}

/* ---------------- Main Priority Scheduling ---------------- */
int main() {
    int n;
    
    double preprocess_start = now_ms();

    Process *p = malloc(sizeof(Process) * MAX);
    
    // Swapping metrics
    int total_page_faults = 0;
    int total_swap_ins = 0;
    int total_swap_outs = 0;
    int memory_frames_used = 0;
    double swap_overhead_time = 0;

    // Load hardcoded data
    hardcoded_data(p, &n);
    
    printf("============ PRIORITY SCHEDULING (Non-Preemptive) ============\n");
    printf("Using hardcoded test data with %d processes\n", n);
    printf("Note: Lower priority number = Higher priority\n");
    printf("\nInput Data:\n");
    printf("Process | AT | BT | Priority | Pages\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("P%-6d | %2d | %2d |    %2d    |  %2d\n", 
               p[i].pid, p[i].arrival, p[i].burst, p[i].priority, p[i].memory_pages);
    }
    printf("\n");

    double preprocess_end = now_ms();
    double schedule_start = now_ms();

    int completed = 0;
    int current_time = 0;
    int total_burst = 0;

    while (completed < n) {
        int idx = -1;
        int highest_priority = 1e9;
        
        for (int i = 0; i < n; i++) {
            if (!p[i].done && p[i].arrival <= current_time) {
                if (p[i].priority < highest_priority ||
                   (p[i].priority == highest_priority && (idx == -1 || p[i].arrival < p[idx].arrival))) {
                    highest_priority = p[i].priority;
                    idx = i;
                }
            }
        }

        if (idx == -1) { // CPU idle
            current_time++;
            continue;
        }

        // Simulate swapping mechanism
        if (!p[idx].in_memory) {
            // Need to swap in
            if (memory_frames_used + p[idx].memory_pages > MEMORY_FRAMES) {
                // Memory full - swap out another process
                for (int j = 0; j < n; j++) {
                    if (p[j].in_memory && j != idx && !p[j].done) {
                        p[j].in_memory = 0;
                        p[j].swap_out_count++;
                        total_swap_outs++;
                        memory_frames_used -= p[j].memory_pages;
                        swap_overhead_time += 0.5;
                        break;
                    }
                }
            }
            
            // Swap in current process
            p[idx].in_memory = 1;
            p[idx].swap_in_count++;
            total_swap_ins++;
            total_page_faults += p[idx].memory_pages;
            memory_frames_used += p[idx].memory_pages;
            swap_overhead_time += 1.0;
        }

        p[idx].start = current_time;
        p[idx].completion = p[idx].start + p[idx].burst;
        p[idx].turnaround = p[idx].completion - p[idx].arrival;
        p[idx].waiting = p[idx].turnaround - p[idx].burst;
        p[idx].response = p[idx].start - p[idx].arrival;

        current_time = p[idx].completion;
        total_burst += p[idx].burst;
        
        // Free memory after completion
        p[idx].in_memory = 0;
        memory_frames_used -= p[idx].memory_pages;
        
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

    // Variance calculation
    double wt_var = 0, tat_var = 0, rt_var = 0;
    for (int i = 0; i < n; i++) {
        wt_var += pow(p[i].waiting - avg_wt, 2);
        tat_var += pow(p[i].turnaround - avg_tat, 2);
        rt_var += pow(p[i].response - avg_rt, 2);
    }
    wt_var /= n;
    tat_var /= n;
    rt_var /= n;

    double wt_std = sqrt(wt_var);
    double tat_std = sqrt(tat_var);
    double rt_std = sqrt(rt_var);

    int first_arrival = p[0].arrival;
    int last_completion = p[0].completion;
    for (int i = 0; i < n; i++) {
        if (p[i].arrival < first_arrival) first_arrival = p[i].arrival;
        if (p[i].completion > last_completion) last_completion = p[i].completion;
    }

    double total_time = last_completion - first_arrival;
    double cpu_util = ((double)total_burst / total_time) * 100.0;
    double throughput = (double)n / total_time;

    /* ---------------- Output ---------------- */
    printf("\n============== PROCESS DETAILS (Priority Scheduling) ==============\n");
    printf("PID | AT | BT | Pri | ST | CT | WT | TAT | RT | Pages | SwapIn | SwapOut\n");
    printf("------------------------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("P%-2d | %2d | %2d |  %2d | %2d | %2d | %2d |  %2d | %2d |   %2d  |   %2d   |   %2d\n",
               p[i].pid, p[i].arrival, p[i].burst, p[i].priority,
               p[i].start, p[i].completion, p[i].waiting,
               p[i].turnaround, p[i].response, p[i].memory_pages,
               p[i].swap_in_count, p[i].swap_out_count);
    }

    printf("\n================ CPU PERFORMANCE METRICS ================\n");
    printf("Total Execution Time    : %.0f units\n", total_time);
    printf("CPU Utilization         : %.2f%%\n", cpu_util);
    printf("Throughput              : %.6f processes/unit\n", throughput);

    printf("\n=============== SCHEDULING TIME METRICS =================\n");
    printf("Average Waiting Time    : %.2f units\n", avg_wt);
    printf("Average Turnaround Time : %.2f units\n", avg_tat);
    printf("Average Response Time   : %.2f units\n", avg_rt);
    printf("Waiting Time Range      : [%d, %d]\n", min_wt, max_wt);
    printf("Turnaround Time Range   : [%d, %d]\n", min_tat, max_tat);
    printf("Response Time Range     : [%d, %d]\n", min_rt, max_rt);

    printf("\n================== VARIANCE ANALYSIS ====================\n");
    printf("Waiting Time     - Variance: %.2f, Std Dev: %.2f\n", wt_var, wt_std);
    printf("Turnaround Time  - Variance: %.2f, Std Dev: %.2f\n", tat_var, tat_std);
    printf("Response Time    - Variance: %.2f, Std Dev: %.2f\n", rt_var, rt_std);

    printf("\n================ MEMORY & SWAPPING METRICS ==============\n");
    printf("Total Page Faults        : %d\n", total_page_faults);
    printf("Total Swap-In Operations : %d\n", total_swap_ins);
    printf("Total Swap-Out Operations: %d\n", total_swap_outs);
    printf("Memory Frames Available  : %d\n", MEMORY_FRAMES);
    printf("Swapping Overhead Time   : %.2f units\n", swap_overhead_time);
    printf("Effective CPU Utilization: %.2f%% (with swapping)\n", 
           ((double)total_burst / (total_time + swap_overhead_time)) * 100);
    printf("Average Swaps per Process: %.2f\n", 
           (double)(total_swap_ins + total_swap_outs) / n);
    printf("Page Fault Rate          : %.3f faults/unit time\n", 
           (double)total_page_faults / total_time);

    printf("\n=================== TIMING BREAKDOWN ====================\n");
    printf("Pre-process time         : %.3f ms\n", preprocess_end - preprocess_start);
    printf("Scheduling time          : %.3f ms\n", schedule_end - schedule_start);
    printf("Total wall time          : %.3f ms\n", schedule_end - preprocess_start);
    printf("=========================================================\n");

    free(p);
    return 0;
}
