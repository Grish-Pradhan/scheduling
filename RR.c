#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define MAX 20
#define PAGE_SIZE 4  // 4 KB pages
#define MEMORY_FRAMES 10  // Limited memory frames

typedef struct {
    int pid;
    int at, bt;
    int priority;
    int rem;
    int ct, tat, wt, rt;
    int started;
    int memory_pages;  // Number of pages required
    int in_memory;     // Whether process is currently in memory
    int swap_in_count;
    int swap_out_count;
} Process;

void hardcoded_data(Process p[], int *n, int *tq) {
    *n = 10;
    *tq = 2;  // Default time quantum for Round Robin
    
    // Hardcoded test data from the image
    int arrival[] = {0, 1, 2, 4, 3, 2, 6, 5, 7, 8};
    int burst[] = {5, 3, 8, 5, 6, 2, 3, 4, 9, 7};
    int priority[] = {1, 3, 5, 6, 8, 9, 7, 5, 6, 3};
    int pages[] = {3, 2, 4, 2, 3, 1, 2, 3, 5, 4};  // Memory pages
    
    for (int i = 0; i < *n; i++) {
        p[i].pid = i + 1;
        p[i].at = arrival[i];
        p[i].bt = burst[i];
        p[i].priority = priority[i];
        p[i].memory_pages = pages[i];
        p[i].rem = p[i].bt;
        p[i].started = 0;
        p[i].in_memory = 0;
        p[i].swap_in_count = 0;
        p[i].swap_out_count = 0;
    }
}

int main() {
    Process p[MAX];
    int n, tq;
    int time = 0, completed = 0;
    int cpu_busy = 0, cpu_idle = 0;
    int context_switches = 0;
    int last_pid = -1;

    // Swapping metrics
    int total_page_faults = 0;
    int total_swap_ins = 0;
    int total_swap_outs = 0;
    int memory_frames_used = 0;
    double swap_overhead_time = 0;

    double wt_sum = 0, tat_sum = 0, rt_sum = 0;

    // Load hardcoded data
    hardcoded_data(p, &n, &tq);
    
    printf("============== ROUND ROBIN SCHEDULING ==============\n");
    printf("Using hardcoded test data with %d processes\n", n);
    printf("Time Quantum: %d\n", tq);
    printf("\nInput Data:\n");
    printf("Process | AT | BT | Priority | Pages\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("P%-6d | %2d | %2d |    %2d    |  %2d\n", 
               p[i].pid, p[i].at, p[i].bt, p[i].priority, p[i].memory_pages);
    }
    printf("\n");

    /* -------- Round Robin Scheduling with Swapping -------- */
    while (completed < n) {
        int progress = 0;

        for (int i = 0; i < n; i++) {
            if (p[i].rem > 0 && p[i].at <= time) {

                // Simulate swapping mechanism
                if (!p[i].in_memory) {
                    // Need to swap in
                    if (memory_frames_used + p[i].memory_pages > MEMORY_FRAMES) {
                        // Memory full - swap out another process
                        for (int j = 0; j < n; j++) {
                            if (p[j].in_memory && j != i && p[j].rem > 0) {
                                p[j].in_memory = 0;
                                p[j].swap_out_count++;
                                total_swap_outs++;
                                memory_frames_used -= p[j].memory_pages;
                                swap_overhead_time += 0.5; // 0.5 time units per swap-out
                                break;
                            }
                        }
                    }
                    
                    // Swap in current process
                    p[i].in_memory = 1;
                    p[i].swap_in_count++;
                    total_swap_ins++;
                    total_page_faults += p[i].memory_pages;
                    memory_frames_used += p[i].memory_pages;
                    swap_overhead_time += 1.0; // 1 time unit per swap-in
                }

                /* Context switch detection */
                if (last_pid != -1 && last_pid != p[i].pid) {
                    context_switches++;
                }

                last_pid = p[i].pid;

                if (!p[i].started) {
                    p[i].rt = time - p[i].at;
                    p[i].started = 1;
                }

                int slice = (p[i].rem > tq) ? tq : p[i].rem;

                time += slice;
                cpu_busy += slice;
                p[i].rem -= slice;
                progress = 1;

                if (p[i].rem == 0) {
                    p[i].ct = time;
                    p[i].tat = p[i].ct - p[i].at;
                    p[i].wt = p[i].tat - p[i].bt;
                    completed++;

                    // Free memory
                    p[i].in_memory = 0;
                    memory_frames_used -= p[i].memory_pages;

                    wt_sum += p[i].wt;
                    tat_sum += p[i].tat;
                    rt_sum += p[i].rt;
                }
            }
        }

        if (!progress) {
            time++;
            cpu_idle++;
        }
    }

    /* -------- Variance Calculation -------- */
    double wt_mean = wt_sum / n;
    double tat_mean = tat_sum / n;
    double rt_mean = rt_sum / n;

    double wt_var = 0, tat_var = 0, rt_var = 0;

    for (int i = 0; i < n; i++) {
        wt_var  += pow(p[i].wt  - wt_mean,  2);
        tat_var += pow(p[i].tat - tat_mean, 2);
        rt_var  += pow(p[i].rt  - rt_mean,  2);
    }

    wt_var  /= n;
    tat_var /= n;
    rt_var  /= n;

    double wt_std = sqrt(wt_var);
    double tat_std = sqrt(tat_var);
    double rt_std = sqrt(rt_var);

    /* -------- Output -------- */
    printf("\n==================== PROCESS DETAILS ====================\n");
    printf("PID  AT  BT  CT  TAT  WT  RT  Pages  SwapIn  SwapOut\n");
    printf("---------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("P%-2d  %2d  %2d  %3d  %3d  %3d  %3d    %2d      %2d      %2d\n",
               p[i].pid, p[i].at, p[i].bt,
               p[i].ct, p[i].tat, p[i].wt, p[i].rt,
               p[i].memory_pages, p[i].swap_in_count, p[i].swap_out_count);
    }

    double cpu_util = ((double)cpu_busy / time) * 100;
    double throughput = (double)n / time;
    double avg_wt = wt_sum / n;
    double avg_tat = tat_sum / n;
    double avg_rt = rt_sum / n;

    printf("\n================ CPU PERFORMANCE METRICS ================\n");
    printf("Total Execution Time: %d time units\n", time);
    printf("CPU Utilization     : %.2f%%\n", cpu_util);
    printf("CPU Idle Time       : %d units\n", cpu_idle);
    printf("CPU Busy Time       : %d units\n", cpu_busy);
    printf("Throughput          : %.3f processes/unit\n", throughput);
    printf("Context Switches    : %d\n", context_switches);

    printf("\n=============== SCHEDULING TIME METRICS =================\n");
    printf("Average Waiting Time    : %.2f\n", avg_wt);
    printf("Average Turnaround Time : %.2f\n", avg_tat);
    printf("Average Response Time   : %.2f\n", avg_rt);

    printf("\n================== VARIANCE ANALYSIS ====================\n");
    printf("Waiting Time     - Variance: %.2f, Std Dev: %.2f\n", wt_var, wt_std);
    printf("Turnaround Time  - Variance: %.2f, Std Dev: %.2f\n", tat_var, tat_std);
    printf("Response Time    - Variance: %.2f, Std Dev: %.2f\n", rt_var, rt_std);

    printf("\n================ MEMORY & SWAPPING METRICS ==============\n");
    printf("Total Page Faults       : %d\n", total_page_faults);
    printf("Total Swap-In Operations: %d\n", total_swap_ins);
    printf("Total Swap-Out Operations: %d\n", total_swap_outs);
    printf("Peak Memory Frames Used : %d / %d\n", MEMORY_FRAMES, MEMORY_FRAMES);
    printf("Swapping Overhead Time  : %.2f units\n", swap_overhead_time);
    printf("Effective CPU Time      : %.2f%% (with swapping overhead)\n", 
           ((double)cpu_busy / (time + swap_overhead_time)) * 100);
    printf("Average Swaps per Process: %.2f\n", (double)(total_swap_ins + total_swap_outs) / n);
    
    double page_fault_rate = (double)total_page_faults / time;
    printf("Page Fault Rate         : %.3f faults/unit time\n", page_fault_rate);
    printf("=========================================================\n");

    return 0;
}
