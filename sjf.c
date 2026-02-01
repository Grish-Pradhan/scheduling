#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX 20
#define PAGE_SIZE 4  // 4 KB pages
#define MEMORY_FRAMES 10  // Limited memory frames

typedef struct {
    int pid;
    int at, bt;
    int priority;
    int start, finish;
    int wt, tat, rt;
    int completed;
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
        p[i].at = arrival[i];
        p[i].bt = burst[i];
        p[i].priority = priority[i];
        p[i].memory_pages = pages[i];
        p[i].completed = 0;
        p[i].in_memory = 0;
        p[i].swap_in_count = 0;
        p[i].swap_out_count = 0;
    }
}

int main() {
    Process p[MAX];
    int n, time = 0, cpu_busy = 0, context_switches = 0;
    
    // Swapping metrics
    int total_page_faults = 0;
    int total_swap_ins = 0;
    int total_swap_outs = 0;
    int memory_frames_used = 0;
    double swap_overhead_time = 0;

    // Load hardcoded data
    hardcoded_data(p, &n);
    
    printf("========== SHORTEST JOB FIRST (SJF) SCHEDULING ==========\n");
    printf("Using hardcoded test data with %d processes\n", n);
    printf("\nInput Data:\n");
    printf("Process | AT | BT | Priority | Pages\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("P%-6d | %2d | %2d |    %2d    |  %2d\n", 
               p[i].pid, p[i].at, p[i].bt, p[i].priority, p[i].memory_pages);
    }
    printf("\n");

    int completed_count = 0;

    while(completed_count < n) {
        // Find next process: shortest job among arrived & incomplete
        int idx = -1;
        int min_bt = 1e9;
        
        for(int i=0; i<n; i++) {
            if(!p[i].completed && p[i].at <= time && p[i].bt < min_bt) {
                min_bt = p[i].bt;
                idx = i;
            }
        }

        if(idx == -1) { // No process has arrived
            time++;
            continue;
        }

        // Simulate swapping mechanism
        if (!p[idx].in_memory) {
            // Need to swap in
            if (memory_frames_used + p[idx].memory_pages > MEMORY_FRAMES) {
                // Memory full - swap out another process
                for (int j = 0; j < n; j++) {
                    if (p[j].in_memory && j != idx && !p[j].completed) {
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

        // Start process
        p[idx].start = time;
        p[idx].rt = p[idx].start - p[idx].at;

        time += p[idx].bt;
        p[idx].finish = time;
        p[idx].tat = p[idx].finish - p[idx].at;
        p[idx].wt = p[idx].tat - p[idx].bt;
        p[idx].completed = 1;

        // Free memory
        p[idx].in_memory = 0;
        memory_frames_used -= p[idx].memory_pages;

        cpu_busy += p[idx].bt;
        completed_count++;
        if(completed_count > 1) context_switches++;
    }

    // Calculate metrics
    double avg_wt=0, avg_tat=0, avg_rt=0;
    int wt_max=0, wt_min=1e9;
    int tat_max=0, tat_min=1e9;
    int rt_max=0, rt_min=1e9;

    for(int i=0; i<n; i++) {
        avg_wt += p[i].wt;
        avg_tat += p[i].tat;
        avg_rt += p[i].rt;
        
        if(p[i].wt > wt_max) wt_max = p[i].wt;
        if(p[i].wt < wt_min) wt_min = p[i].wt;
        if(p[i].tat > tat_max) tat_max = p[i].tat;
        if(p[i].tat < tat_min) tat_min = p[i].tat;
        if(p[i].rt > rt_max) rt_max = p[i].rt;
        if(p[i].rt < rt_min) rt_min = p[i].rt;
    }

    avg_wt /= n; 
    avg_tat /= n; 
    avg_rt /= n;

    // Variance calculation
    double wt_var = 0, tat_var = 0, rt_var = 0;
    for(int i=0; i<n; i++) {
        wt_var += pow(p[i].wt - avg_wt, 2);
        tat_var += pow(p[i].tat - avg_tat, 2);
        rt_var += pow(p[i].rt - avg_rt, 2);
    }
    wt_var /= n;
    tat_var /= n;
    rt_var /= n;

    double wt_std = sqrt(wt_var);
    double tat_std = sqrt(tat_var);
    double rt_std = sqrt(rt_var);

    double throughput = (double)n / time;
    double cpu_util = ((double)cpu_busy / time) * 100;

    // Display table
    printf("\n==================== PROCESS DETAILS (SJF) ====================\n");
    printf("PID | AT | BT | Start | Finish | WT | TAT | RT | Pages | SwapIn | SwapOut\n");
    printf("--------------------------------------------------------------------\n");
    for(int i=0; i<n; i++) {
        printf("P%-2d | %2d | %2d |  %3d  |  %4d  | %2d | %3d | %2d |   %2d  |   %2d   |   %2d\n",
               p[i].pid, p[i].at, p[i].bt, p[i].start, p[i].finish,
               p[i].wt, p[i].tat, p[i].rt, p[i].memory_pages,
               p[i].swap_in_count, p[i].swap_out_count);
    }

    printf("\n================ CPU PERFORMANCE METRICS ================\n");
    printf("Total Execution Time    : %d units\n", time);
    printf("CPU Utilization         : %.2f%%\n", cpu_util);
    printf("Throughput              : %.3f processes/unit\n", throughput);
    printf("Context Switches        : %d\n", context_switches);

    printf("\n=============== SCHEDULING TIME METRICS =================\n");
    printf("Average Waiting Time    : %.2f units\n", avg_wt);
    printf("Average Turnaround Time : %.2f units\n", avg_tat);
    printf("Average Response Time   : %.2f units\n", avg_rt);
    printf("Maximum Waiting Time    : %d units\n", wt_max);
    printf("Minimum Waiting Time    : %d units\n", wt_min);
    printf("Maximum Turnaround Time : %d units\n", tat_max);
    printf("Minimum Turnaround Time : %d units\n", tat_min);
    printf("Maximum Response Time   : %d units\n", rt_max);
    printf("Minimum Response Time   : %d units\n", rt_min);

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
           ((double)cpu_busy / (time + swap_overhead_time)) * 100);
    printf("Average Swaps per Process: %.2f\n", 
           (double)(total_swap_ins + total_swap_outs) / n);
    printf("Page Fault Rate          : %.3f faults/unit time\n", 
           (double)total_page_faults / time);
    printf("=========================================================\n");

    return 0;
}
