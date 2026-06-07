#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define MAX_TASKS 7
#define MAX_TIME 1000

typedef struct {
    int task_id;
    int execution_time;
    int period;
    int deadline;
    int remaining_time;
    int priority;
    int status; // 0=ready, 1=running, 2=blocked
    int start_time;
    int end_time;
} Task;

typedef struct {
    int num_tasks;
    Task tasks[MAX_TASKS];
    float utilization;
} TaskSet;

typedef struct {
    int time;
    int task_id;
    char event[50]; // "start", "end", "preempt"
} Event;

Event events[MAX_TIME];
int event_count = 0;

// Read task set from input file
void read_taskset(const char *filename, TaskSet *taskset) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }

    fscanf(fp, "%d", &taskset->num_tasks);
    taskset->utilization = 0.0;

    for (int i = 0; i < taskset->num_tasks; i++) {
        taskset->tasks[i].task_id = i + 1;
        fscanf(fp, "%d %d",
            &taskset->tasks[i].execution_time,
            &taskset->tasks[i].period);

        taskset->tasks[i].deadline = taskset->tasks[i].period;
        taskset->tasks[i].remaining_time = taskset->tasks[i].execution_time;
        taskset->tasks[i].status = 0; // ready

        float util = (float)taskset->tasks[i].execution_time /
                    (float)taskset->tasks[i].period;
        taskset->utilization += util;
    }

    fclose(fp);
}

// Rate Monotonic - sort by period (smaller period = higher priority)
void rm_assign_priority(TaskSet *taskset) {
    for (int i = 0; i < taskset->num_tasks; i++) {
        taskset->tasks[i].priority = taskset->tasks[i].period;
    }
}

// EDF - sort by deadline (for dynamic scheduling)
void edf_assign_priority(TaskSet *taskset) {
    for (int i = 0; i < taskset->num_tasks; i++) {
        taskset->tasks[i].priority = taskset->tasks[i].deadline;
    }
}

// Find next ready task with highest priority (lowest priority value)
int select_next_task(TaskSet *taskset) {
    int best_idx = -1;
    int best_priority = INT_MAX;

    for (int i = 0; i < taskset->num_tasks; i++) {
        if (taskset->tasks[i].status == 0 &&
            taskset->tasks[i].priority < best_priority) {
            best_priority = taskset->tasks[i].priority;
            best_idx = i;
        }
    }

    return best_idx;
}

// Generate task arrival times (periodic)
void generate_arrivals(TaskSet *taskset, int total_time, int *arrivals[MAX_TASKS]) {
    for (int i = 0; i < taskset->num_tasks; i++) {
        arrivals[i] = (int *)malloc(sizeof(int) * (total_time + 1));
        for (int t = 0; t <= total_time; t++) {
            if (t % taskset->tasks[i].period == 0 && t > 0) {
                arrivals[i][t] = 1;
            } else {
                arrivals[i][t] = 0;
            }
        }
    }
}

// Simulate RM or EDF scheduling
void simulate_scheduling(TaskSet *taskset, int total_time, int algorithm) {
    if (algorithm == 0) {
        printf("=== RATE MONOTONIC (RM) SCHEDULING ===\n");
        rm_assign_priority(taskset);
    } else {
        printf("=== EARLIEST DEADLINE FIRST (EDF) SCHEDULING ===\n");
        edf_assign_priority(taskset);
    }

    printf("Task Utilization: %.2f%%\n", taskset->utilization * 100);
    printf("\nTask Information:\n");
    for (int i = 0; i < taskset->num_tasks; i++) {
        printf("Task %d: Execution Time=%d, Period=%d, Utilization=%.2f%%\n",
            taskset->tasks[i].task_id,
            taskset->tasks[i].execution_time,
            taskset->tasks[i].period,
            (float)taskset->tasks[i].execution_time / taskset->tasks[i].period * 100);
    }

    printf("\n--- Scheduling Trace ---\n");
    printf("Time | Running Task | Event\n");
    printf("-----|--------------|-------------------\n");

    int current_task = -1;
    int time_slice = 0;

    for (int t = 0; t < total_time; t++) {
        // Check for new arrivals (task releases)
        for (int i = 0; i < taskset->num_tasks; i++) {
            if (t > 0 && t % taskset->tasks[i].period == 0) {
                taskset->tasks[i].status = 0; // ready
                taskset->tasks[i].remaining_time = taskset->tasks[i].execution_time;
                taskset->tasks[i].deadline = t + taskset->tasks[i].period;
                printf("%4d | Task%d        | Released\n", t, taskset->tasks[i].task_id);
            }
        }

        // Select next task
        int next_task = select_next_task(taskset);

        if (next_task != current_task) {
            if (current_task >= 0) {
                printf("%4d | Task%d        | Preempted\n", t,
                    taskset->tasks[current_task].task_id);
            }
            if (next_task >= 0) {
                printf("%4d | Task%d        | Started\n", t,
                    taskset->tasks[next_task].task_id);
            }
            current_task = next_task;
        }

        // Execute current task
        if (current_task >= 0) {
            taskset->tasks[current_task].status = 1; // running
            taskset->tasks[current_task].remaining_time--;

            if (taskset->tasks[current_task].remaining_time == 0) {
                printf("%4d | Task%d        | Completed\n", t,
                    taskset->tasks[current_task].task_id);
                taskset->tasks[current_task].status = 2; // blocked/waiting
                current_task = -1;
            }
        } else {
            printf("%4d | None         | Idle\n", t);
        }
    }
}

// Main function
int main(int argc, char *argv[]) {
    TaskSet taskset;
    char input_file[256];

    if (argc < 2) {
        strcpy(input_file, "Input.txt");
    } else {
        strcpy(input_file, argv[1]);
    }

    printf("Task Scheduling Simulator\n");
    printf("========================\n\n");

    read_taskset(input_file, &taskset);

    if (taskset.num_tasks == 0) {
        printf("No tasks loaded.\n");
        return 1;
    }

    // Validate utilization
    if (taskset.utilization > 0.65) {
        printf("ERROR: Total utilization %.2f%% exceeds 65%% limit!\n",
            taskset.utilization * 100);
        return 1;
    }

    // Simulate RM scheduling
    simulate_scheduling(&taskset, 200, 0);

    printf("\n\n");

    // Reset and simulate EDF scheduling
    read_taskset(input_file, &taskset);
    simulate_scheduling(&taskset, 200, 1);

    return 0;
}
