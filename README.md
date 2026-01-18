# CPU Scheduling Algorithms in C

This repository contains a **C program** that simulates and analyzes various CPU scheduling algorithms. It allows users to enter processes with their **arrival time**, **burst time**, and **priority**, then calculates scheduling metrics such as **waiting time**, **turnaround time**, and **response time**. The program supports multiple scheduling strategies and works on both **Windows** and **Linux** environments.

## Features

The program implements the following CPU scheduling algorithms:

1. **First-Come, First-Served (FCFS)**

   * Non-preemptive scheduling based on arrival order.
   * Calculates start, finish, waiting, turnaround, and response times for each process.

2. **Shortest Job First (SJF)**

   * Non-preemptive scheduling based on the shortest burst time.
   * Prioritizes processes with shorter execution times.

3. **Round Robin (RR)**

   * Preemptive scheduling with a user-defined **time quantum**.
   * Processes are executed in a cyclic order.

4. **Priority Scheduling (PS)**

   * Non-preemptive scheduling based on priority values.
   * Lower priority numbers indicate higher priority.

## Program Highlights

* Interactive input for process details: **PID**, **arrival time**, **burst time**, **priority**.
* Automatic calculation of:

  * **Start Time (ST)**
  * **Finish Time (FT)**
  * **Waiting Time (WT)**
  * **Turnaround Time (TAT)**
  * **Response Time (RT)**
* Computes **average metrics** for performance comparison.
* High-resolution timers to simulate real execution time (works on **Windows** and **Linux**).

## Requirements

* C compiler:

  * GCC (`gcc`) for Linux/macOS
  * MinGW or similar for Windows
* Terminal or Command Prompt for execution

## Usage

1. Clone the repository:

```bash
git clone https://github.com/Grish-Pradhan/scheduling.git
cd scheduling
```

2. Compile the program:

```bash
gcc scheduling.c -o scheduling   # Linux
```

or for Windows (PowerShell/Command Prompt):

```bash
gcc scheduling.c -o scheduling.exe
```

3. Run the program:

```bash
./scheduling        # Linux
scheduling.exe      # Windows
```

4. Follow the prompts to:

   * Enter the number of processes
   * Enter arrival time, burst time, and priority for each process
   * Select the scheduling algorithm to simulate

5. View the results, including per-process metrics and average values.


## Contributing

Contributions are welcome! You can:

* Add **preemptive SJF**
* Add **Gantt chart visualization**
* Improve **input validation**
* Optimize algorithms for larger datasets

## License

This project is licensed under the **MIT License**. 
