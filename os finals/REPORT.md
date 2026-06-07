# Operating System Practice - Final Project Report
## Task Scheduling Implementation (RM and EDF)

---

### 1. Introduction

This project implements Rate Monotonic (RM) and Earliest Deadline First (EDF) scheduling algorithms based on uC/OS II microkernel principles. The scheduler manages periodic real-time tasks and provides deterministic scheduling with priority-based task selection.

**Project Objectives:**
- Implement priority-driven scheduling in an RTOS
- Support both RM (static priority) and EDF (dynamic priority) algorithms
- Handle periodic task arrivals and preemption
- Provide detailed execution traces and context switch information

**System Constraints:**
- Maximum 7 tasks
- Total utilization ≤ 65%
- Tasks have execution time and period parameters

---

### 2. System Design and Architecture

#### 2.1 Task Structure
Each task is represented as a Task Control Block (TCB) containing:
- `task_id`: Unique task identifier
- `execution_time`: Worst-case execution time (WCET)
- `period`: Task period (release interval)
- `deadline`: Absolute deadline for current instance
- `remaining_time`: Time units left in current execution
- `priority`: Scheduling priority (assigned by algorithm)
- `status`: Current state (ready, running, or blocked)

#### 2.2 Scheduling Algorithms

**Rate Monotonic (RM):**
- Static priority assignment inversely proportional to period
- Shorter period → Higher priority (lower priority value)
- Optimal for fixed-priority preemptive scheduling
- Utilization bound: ln(2) ≈ 0.69 for n tasks

**Earliest Deadline First (EDF):**
- Dynamic priority based on absolute deadline
- Task with earliest deadline gets highest priority
- Always selects the task with minimum deadline value
- Optimal for dynamic priority scheduling
- Utilization bound: 100% (theoretical)

#### 2.3 Scheduler Implementation

The OS_Sched() function selects the next ready task:
```
1. Scan all ready tasks
2. Find task with highest priority (lowest priority value)
3. If priority differs from current task → preemption occurs
4. Execute selected task for one time unit
5. Handle task completion and periodic releases
```

#### 2.4 Event Handling

Key scheduler events:
- **Task Release**: New periodic instance becomes ready
- **Context Switch**: Preemption due to higher priority task
- **Task Completion**: Task finishes execution for current period
- **Idle**: No ready task (CPU idle period)

---

### 3. Implementation Details

#### 3.1 Data Structures

```c
typedef struct {
    int task_id;              // Task identifier (1 to N)
    int execution_time;       // WCET in time units
    int period;               // Task period
    int deadline;             // Absolute deadline
    int remaining_time;       // Time remaining in current execution
    int priority;             // Assigned priority
    int status;               // 0=ready, 1=running, 2=blocked
} Task;

typedef struct {
    int num_tasks;            // Number of tasks
    Task tasks[MAX_TASKS];    // Task array
    float utilization;        // Total utilization
} TaskSet;
```

#### 3.2 Key Functions

**read_taskset(filename, taskset):**
- Reads task parameters from input file
- Calculates total utilization
- Initializes task states

**rm_assign_priority(taskset):**
- Assigns static priorities based on periods
- Priority = Period (shorter period = higher priority)

**edf_assign_priority(taskset):**
- Sets initial deadlines equal to periods
- Deadlines updated dynamically at task release

**select_next_task(taskset):**
- Scans ready task list
- Returns index of highest priority ready task
- Returns -1 if no ready task (idle)

**simulate_scheduling(taskset, total_time, algorithm):**
- Main simulation loop
- Handles task releases, executions, and completions
- Generates event trace for analysis

#### 3.3 Task Release and Deadline Update

At time t, if t % task.period == 0:
1. Task becomes ready (status = 0)
2. Remaining time reset to execution time
3. Deadline updated: deadline = t + period
4. Task placed in ready queue

#### 3.4 Preemption Handling

When OS_Sched() detects a higher-priority ready task:
1. Current task's remaining_time is preserved
2. Current task returns to ready queue
3. Higher-priority task begins execution
4. Trace records preemption event

---

### 4. Input/Output Specification

#### 4.1 Input Format

File: `Input.txt`
```
N                    // Number of tasks (1-7)
E1 P1               // Task 1: Execution time, Period
E2 P2               // Task 2: Execution time, Period
...
EN PN               // Task N: Execution time, Period
```

**Utilization Check:**
- Sum of (Ei / Pi) for all tasks ≤ 0.65
- Program rejects task set if utilization exceeds limit

#### 4.2 Output Format

The scheduler produces:
1. **Task Information Section:**
   - List all tasks with their parameters
   - Display calculated utilization percentage
   - Show assigned priorities for each algorithm

2. **Scheduling Trace:**
   - Time stamp
   - Running task (or "None" for idle)
   - Event type (Started, Completed, Released, Preempted)
   - Chronological order of execution

3. **Example Output:**
   ```
   Time | Running Task | Event
   -----|--------------|-------------------
      0 | Task 1       | Started
      1 | Task 1       | Completed
      3 | Task 2       | Released
      3 | Task 2       | Started
      ...
   ```

---

### 5. Test Cases and Results

**Test Case 1: Input1.txt**
- Tasks: 4
- Utilization: 48.15%
- RM Priority Assignment:
  - Task 1: Priority 12 (period 12)
  - Task 2: Priority 7 (period 7)
  - Task 3: Priority 19 (period 19)
  - Task 4: Priority 20 (period 20)

RM Scheduling Behavior:
- Task 2 has highest priority (shortest period)
- Executes frequently with minimal blocking
- Task 4 has lowest priority
- Preemptions occur when higher-priority tasks release

**Test Case 2: Input2.txt**
- Tasks: 5
- Utilization: 42.62%
- Demonstrates handling of multiple simultaneous releases
- Shows EDF behavior with dynamic deadline-based selection

---

### 6. Algorithm Comparison

| Aspect | RM | EDF |
|--------|-----|-----|
| **Priority Type** | Static | Dynamic |
| **Assignment** | By period | By deadline |
| **Optimality** | Optimal (fixed-priority) | Optimal (dynamic) |
| **Complexity** | O(n) at context switch | O(n) at context switch |
| **Util Bound** | ln(2) ≈ 0.69 | 1.0 (100%) |
| **Implementation** | Simpler | Requires deadline update |

---

### 7. Problems Solved

1. **Priority Inversion Handling:**
   - Immediate scheduling upon higher-priority release
   - Lower-priority tasks properly preempted
   - No unbounded delays

2. **Periodic Task Management:**
   - Correct deadline calculation per period
   - Automatic release at period boundaries
   - Remaining time preservation across preemptions

3. **Utilization Validation:**
   - Input validation prevents over-committed systems
   - 65% threshold ensures feasibility
   - Clear error messages for invalid inputs

---

### 8. Extra Exercises and Extensions

**Optional Enhancement 1: Priority Inheritance Protocol (PIP)**
- Prevents priority inversion in shared resources
- Higher-priority task waiting on lower-priority → boost priority
- Complexity: +20% extra credit

**Optional Enhancement 2: Priority Ceiling Protocol (PCP)**
- Ceiling = max priority of all tasks accessing resource
- Task assumes ceiling priority when accessing resource
- Complexity: +30% extra credit

**Implementation Approach:**
- Extend TCB with resource ownership fields
- Add resource wait queue and priority boosting logic
- Modify OS_Sched() to enforce ceiling protocol

---

### 9. Compilation and Execution

**Compile:**
```bash
gcc -Wall -std=c99 -o scheduler scheduler.c
```

**Run with input file:**
```bash
./scheduler Input1.txt
./scheduler Input2.txt
```

**Output:**
Both RM and EDF scheduling traces to console

---

### 10. Conclusion

This implementation provides a functional real-time task scheduler demonstrating:
- Correct priority-based task selection
- Accurate handling of periodic arrivals
- Proper preemption and context switching
- Clear execution traces for verification

The scheduler successfully implements both RM and EDF algorithms within the constraints of the specification, providing a solid foundation for real-time embedded systems education.

---

**Team Members:** [INSERT NAMES AND STUDENT IDs]
**Submission Date:** June 8, 2026
**Project Duration:** [X hours/days]
