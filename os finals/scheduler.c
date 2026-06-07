#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define MAX_TASKS 7
#define MAX_TIME 1000
#define MAX_JOBS 100

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
    int job_id;
    int task_id;
    int period;
    int execution_time;
    int release_time;
    int absolute_deadline;
    int start_time;
    int end_time;
    int completed;
    char status[20];
} JobRecord;

typedef struct {
    int time;
    int task_id;
    char event[50]; // "start", "end", "preempt"
} Event;

Event events[MAX_TIME];
int event_count = 0;

JobRecord jobs[MAX_JOBS];
int job_count = 0;

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

// Forward declarations
void record_job(int task_id, int period, int exec_time, int release,
                int deadline, int start, int end, int completed);
void print_job_table(const char *algorithm_name);

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

// Simulate RM or EDF scheduling with job tracking
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
    int task_start_time = -1;
    int job_instance[MAX_TASKS];
    for (int i = 0; i < taskset->num_tasks; i++) job_instance[i] = 1; // Start at 1 for correct calculation

    for (int t = 0; t < total_time; t++) {
        // Check for new arrivals (task releases)
        for (int i = 0; i < taskset->num_tasks; i++) {
            if (t > 0 && t % taskset->tasks[i].period == 0) {
                taskset->tasks[i].status = 0; // ready
                taskset->tasks[i].remaining_time = taskset->tasks[i].execution_time;
                taskset->tasks[i].deadline = t + taskset->tasks[i].period;
                job_instance[i]++;
                printf("%4d | Task%d        | Released (Job %d)\n", t,
                    taskset->tasks[i].task_id, job_instance[i]);
            } else if (t == 0) {
                // Initialize all tasks as ready at t=0
                taskset->tasks[i].status = 0;
                taskset->tasks[i].remaining_time = taskset->tasks[i].execution_time;
                taskset->tasks[i].deadline = t + taskset->tasks[i].period;
            }
        }

        // Select next task
        int next_task = select_next_task(taskset);

        // Only preempt if next_task has HIGHER priority (LOWER priority value)
        int should_switch = 0;
        if (next_task != current_task) {
            if (current_task < 0) {
                // No current task, switch to next if available
                should_switch = (next_task >= 0) ? 1 : 0;
            } else if (next_task >= 0) {
                // Compare priorities: lower value = higher priority
                if (taskset->tasks[next_task].priority < taskset->tasks[current_task].priority) {
                    should_switch = 1; // Higher priority task ready, preempt
                }
            }

            if (should_switch) {
                if (current_task >= 0) {
                    printf("%4d | Task%d        | Preempted\n", t,
                        taskset->tasks[current_task].task_id);
                }
                printf("%4d | Task%d        | Started (Job %d)\n", t,
                    taskset->tasks[next_task].task_id, job_instance[next_task]);
                task_start_time = t;
                current_task = next_task;
            }
        }

        // Execute current task
        if (current_task >= 0) {
            taskset->tasks[current_task].status = 1; // running
            taskset->tasks[current_task].remaining_time--;

            if (taskset->tasks[current_task].remaining_time == 0) {
                int task_idx = current_task;
                int end_time = t + 1;

                // Calculate correct release time based on job instance
                int release_time = (job_instance[task_idx] - 1) * taskset->tasks[task_idx].period;
                int deadline = release_time + taskset->tasks[task_idx].period;

                printf("%4d | Task%d        | Completed (Job %d)\n", t,
                    taskset->tasks[task_idx].task_id, job_instance[task_idx]);

                // Record job execution
                record_job(
                    taskset->tasks[task_idx].task_id,
                    taskset->tasks[task_idx].period,
                    taskset->tasks[task_idx].execution_time,
                    release_time,
                    deadline,
                    task_start_time,
                    end_time,
                    1
                );

                taskset->tasks[task_idx].status = 2; // blocked/waiting
                current_task = -1;
            }
        } else {
            printf("%4d | None         | Idle\n", t);
        }
    }

    // Print job execution table
    print_job_table(algorithm == 0 ? "RATE MONOTONIC (RM)" : "EARLIEST DEADLINE FIRST (EDF)");

    // Print task summary table
    printf("\n");
    printf("================================================================================\n");
    printf("TASK SUMMARY TABLE - %s\n", algorithm == 0 ? "RATE MONOTONIC (RM)" : "EARLIEST DEADLINE FIRST (EDF)");
    printf("================================================================================\n");
    printf("P = period  |  C = execution  |  D = absolute deadline\n");
    printf("================================================================================\n");

    for (int i = 0; i < taskset->num_tasks; i++) {
        int first_start = INT_MAX;
        int last_end = 0;
        int job_cnt = 0;

        // Find actual start/end times and job count from execution
        for (int j = 0; j < job_count; j++) {
            if (jobs[j].task_id == taskset->tasks[i].task_id) {
                if (jobs[j].start_time < first_start) {
                    first_start = jobs[j].start_time;
                }
                if (jobs[j].end_time > last_end) {
                    last_end = jobs[j].end_time;
                }
                job_cnt++;
            }
        }

        if (first_start == INT_MAX) first_start = 0;
        if (last_end == 0) last_end = 0;

        printf("Task%d  P=%d  C=%d  D=%d  start=%d  end=%d  jobs=%d\n",
            taskset->tasks[i].task_id,
            taskset->tasks[i].period,
            taskset->tasks[i].execution_time,
            taskset->tasks[i].period,
            first_start,
            last_end,
            job_cnt
        );
    }
    printf("================================================================================\n");

    // Reset job counter for next algorithm
    job_count = 0;
}

// Track job execution details
void record_job(int task_id, int period, int exec_time, int release,
                int deadline, int start, int end, int completed) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].job_id = job_count + 1;
        jobs[job_count].task_id = task_id;
        jobs[job_count].period = period;
        jobs[job_count].execution_time = exec_time;
        jobs[job_count].release_time = release;
        jobs[job_count].absolute_deadline = deadline;
        jobs[job_count].start_time = start;
        jobs[job_count].end_time = end;
        jobs[job_count].completed = completed;

        if (completed && end <= deadline) {
            strcpy(jobs[job_count].status, "✓ Met");
        } else if (completed && end > deadline) {
            strcpy(jobs[job_count].status, "✗ MISSED");
        } else {
            strcpy(jobs[job_count].status, "Incomplete");
        }

        job_count++;
    }
}

// Print job execution table
void print_job_table(const char *algorithm_name) {
    printf("\n");
    printf("================================================================================\n");
    printf("%s - DETAILED JOB EXECUTION TABLE\n", algorithm_name);
    printf("================================================================================\n");
    printf("JOB# | TASK | PERIOD | EXEC | RELEASE | DEADLINE | START | END  | STATUS\n");
    printf("-----|------|--------|------|---------|----------|-------|------|---------------\n");

    for (int i = 0; i < job_count; i++) {
        printf("%4d | T%-2d  | %6d | %4d | %7d | %8d | %5d | %4d | %s\n",
            jobs[i].job_id,
            jobs[i].task_id,
            jobs[i].period,
            jobs[i].execution_time,
            jobs[i].release_time,
            jobs[i].absolute_deadline,
            jobs[i].start_time,
            jobs[i].end_time,
            jobs[i].status);
    }
    printf("================================================================================\n\n");
}

// Print context switches summary
void print_context_switches() {
    printf("================================================================================\n");
    printf("CONTEXT SWITCH ANALYSIS\n");
    printf("================================================================================\n");

    int switch_count = 0;
    int last_task = -1;

    for (int i = 0; i < job_count; i++) {
        if (jobs[i].task_id != last_task && jobs[i].start_time >= 0) {
            switch_count++;
            printf("Switch %d: Time %d - Task %d starts (Released at %d, Deadline %d)\n",
                switch_count,
                jobs[i].start_time,
                jobs[i].task_id,
                jobs[i].release_time,
                jobs[i].absolute_deadline);
            last_task = jobs[i].task_id;
        }
    }

    printf("\nTotal Context Switches: %d\n", switch_count);
    printf("================================================================================\n\n");
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

    // Simulate RM scheduling (1000 time units to allow all tasks to complete)
    simulate_scheduling(&taskset, 1000, 0);

    printf("\n\n");

    // Reset and simulate EDF scheduling
    read_taskset(input_file, &taskset);
    simulate_scheduling(&taskset, 1000, 1);

    printf("\n");
    printf("================================================================================\n");
    printf("FINAL SUMMARY\n");
    printf("================================================================================\n");
    printf("Task Set Utilization: %.2f%%\n", taskset.utilization * 100);
    printf("Utilization Status: %s (≤ 65%% required)\n",
        taskset.utilization <= 0.65 ? "✓ VALID" : "✗ INVALID");
    printf("Number of Tasks: %d (max 7)\n", taskset.num_tasks);
    printf("Simulation Time Window: 0-199 time units\n");
    printf("================================================================================\n");

    return 0;
}
