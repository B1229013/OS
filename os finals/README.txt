================================================================================
OPERATING SYSTEM FINAL PROJECT - TASK SCHEDULING
Rate Monotonic (RM) and Earliest Deadline First (EDF) Implementation
================================================================================

PROJECT OVERVIEW
================================================================================

This project implements a task scheduler demonstrating Rate Monotonic and
Earliest Deadline First scheduling algorithms for real-time operating systems.

The scheduler:
- Reads periodic task sets from input files
- Assigns priorities based on RM or EDF algorithms
- Simulates task execution over time
- Generates detailed execution traces showing all context switches
- Validates task set utilization constraints


FILES INCLUDED
================================================================================

SOURCE CODE:
  scheduler.c              - Complete scheduler implementation (~500 lines)
                            Implements both RM and EDF algorithms
                            Handles periodic task arrivals and preemption
                            Generates detailed execution traces

INPUT FILES:
  Input1.txt               - Test case 1: 4 tasks, 48.15% utilization
  Input2.txt               - Test case 2: 5 tasks, 42.62% utilization

DOCUMENTATION:
  README.txt               - This file
  REPORT.md                - Comprehensive project report (4 pages)
  IMPLEMENTATION_GUIDE.txt - Detailed implementation guide

EXECUTABLE:
  scheduler.exe            - Compiled Windows executable (ready to run)

OUTPUT EXAMPLE:
  output_RM_EDF_Input1.txt - Sample output from Input1.txt


QUICK START
================================================================================

Windows:
  C:\> cd "C:\Users\user\Desktop\school\os\os finals"
  C:\> scheduler.exe Input1.txt
  C:\> scheduler.exe Input2.txt

Linux/Mac (requires gcc):
  $ gcc -Wall -std=c99 -o scheduler scheduler.c
  $ ./scheduler Input1.txt
  $ ./scheduler Input2.txt


HOW TO USE
================================================================================

1. Prepare input file with task set:
   - First line: number of tasks (1-7)
   - Each following line: execution_time period
   - Total utilization must not exceed 65%

2. Run scheduler:
   $ scheduler.exe your_input_file.txt

3. Read output:
   - Task Information: Lists loaded tasks and priorities
   - RM Scheduling Trace: Shows execution sequence with RM algorithm
   - EDF Scheduling Trace: Shows execution sequence with EDF algorithm
   - Each event marked with time, running task, and event type


UNDERSTANDING TASK SET FORMAT
================================================================================

Example task set (3 tasks):
  3                    <- Number of tasks
  1 5                  <- Task 1: 1 time unit execution, 5 time units period
  2 9                  <- Task 2: 2 time units execution, 9 time units period
  3 12                 <- Task 3: 3 time units execution, 12 time units period

Utilization = (1/5) + (2/9) + (3/12) = 0.20 + 0.22 + 0.25 = 0.67 = 67%
This exceeds 65% limit, so it would be REJECTED.

Valid utilization: ≤ 65%


KEY CONCEPTS
================================================================================

RATE MONOTONIC (RM):
- Assigns static priority inversely proportional to task period
- Task with shorter period gets higher priority (executes more often)
- Theoretically optimal for fixed-priority preemptive scheduling
- All demonstrated task sets pass RM schedulability test

EARLIEST DEADLINE FIRST (EDF):
- Dynamically assigns priority based on absolute deadline
- Always executes task with earliest deadline
- Can achieve 100% utilization theoretically
- More powerful than RM but requires dynamic priority adjustment

PREEMPTION:
- When higher-priority task becomes ready, lower-priority task pauses
- Lower-priority task resumes when higher-priority work completes
- Remaining execution time is preserved

CONTEXT SWITCH:
- Change from one running task to another
- Recorded in output as "Preempted" or transition to new task
- Zero overhead in this simulation


SAMPLE OUTPUT INTERPRETATION
================================================================================

Time | Running Task | Event
-----|--------------|-------------------
   0 | Task2        | Started      <- Task 2 starts executing
   0 | Task2        | Completed    <- Task 2 finishes (executed 0-1)
   1 | Task1        | Started      <- Task 1 starts
   1 | Task1        | Completed    <- Task 1 finishes
   3 | Task2        | Released     <- Task 2 periodic release (at t=7)
   3 | Task2        | Started      <- Task 2 starts again


ALGORITHM COMPARISON OUTPUT
================================================================================

The scheduler outputs BOTH algorithms in sequence:

1. RATE MONOTONIC (RM) SCHEDULING
   - Shows RM priority assignments
   - Complete execution trace using RM algorithm
   - Task priorities based on periods

2. EARLIEST DEADLINE FIRST (EDF) SCHEDULING
   - Shows EDF deadline-based selection
   - Complete execution trace using EDF algorithm
   - Task priorities based on absolute deadlines


TESTING & VALIDATION
================================================================================

The scheduler validates:
1. Task count is between 1 and 7
2. Total utilization does not exceed 65%
3. All execution times and periods are positive integers

The implementation correctly:
1. Releases tasks at period boundaries (t = k * period)
2. Maintains task remaining execution time across preemptions
3. Updates deadlines when tasks are released
4. Selects highest-priority ready task
5. Records all context switches in trace


CUSTOMIZATION
================================================================================

To add your own test cases:

1. Create text file with task set:
   - Line 1: number of tasks
   - Lines 2+: execution_time period (space-separated)

2. Run scheduler with your file:
   $ scheduler.exe your_tasks.txt

3. Verify utilization is acceptable
4. Check output for correct scheduling behavior


PRESENTATION PREPARATION
================================================================================

5-minute presentation structure:
1. Show task set from input file (0:30)
2. Explain RM vs EDF differences (1:00)
3. Run scheduler and interpret output (2:00)
4. Discuss scheduling decisions and preemptions (1:30)

Key points to highlight:
- How RM assigns static priorities by period
- How EDF dynamically selects by deadline
- Differences in execution order between algorithms
- Importance of preemption in meeting deadlines
- Why higher priority tasks execute more frequently


SUBMISSION REQUIREMENTS
================================================================================

Create OSP-Project-X.zip (X = team number) containing:
  ☐ scheduler.c (source code)
  ☐ Input1.txt, Input2.txt (test cases)
  ☐ REPORT.md or REPORT.docx (project report)
  ☐ This README.txt or equivalent documentation
  ☐ Team member names and student IDs in report

Optional extras:
  ☐ Additional test cases
  ☐ Performance analysis
  ☐ PIP/PCP implementation (bonus)


COMPILATION NOTES
================================================================================

Compiled with:
  gcc version 15.2.0 (mingw)
  Standard: C99
  Flags: -Wall (all warnings)

For other compilers:
  - Any standard C99 compiler should work
  - No special libraries required (only standard C)
  - Portable across Windows, Linux, Mac


TROUBLESHOOTING
================================================================================

"Cannot open file":
  - Ensure input file exists in current directory
  - Check filename spelling
  - Try full path: scheduler.exe C:\path\to\Input1.txt

"Utilization exceeds 65%":
  - Task set is overloaded
  - Reduce task execution times or increase periods
  - Refer to example input files for valid utilizations

No output or crashes:
  - Verify input file format is correct
  - Check first line contains valid task count (1-7)
  - Each subsequent line must have two integers separated by space

Different output than expected:
  - Verify the scheduler is reading correct input file
  - Check that task set parameters match input file
  - See output_RM_EDF_Input1.txt for comparison


REFERENCES
================================================================================

For understanding the algorithms:
- RM Schedulability: Total util ≤ n(2^(1/n) - 1)
- EDF Schedulability: Total util ≤ 100%
- uC/OS II Reference: Included implementation concepts
- Real-time Systems: Course material on scheduling theory

Key implementation functions in scheduler.c:
- read_taskset(): Input file parsing
- rm_assign_priority(): RM static priority assignment
- select_next_task(): Core scheduling decision
- simulate_scheduling(): Main simulation loop


ADDITIONAL NOTES
================================================================================

This implementation demonstrates:
✓ Priority-based preemptive scheduling
✓ Correct handling of periodic task arrivals
✓ Proper preemption and context switching
✓ Real-time task management concepts
✓ Deterministic scheduling behavior

The scheduler is suitable for:
- Operating systems course projects
- Real-time systems education
- Understanding scheduler behavior
- Comparing different scheduling algorithms


QUESTIONS & SUPPORT
================================================================================

For questions about the project, refer to:
1. REPORT.md - Detailed technical documentation
2. IMPLEMENTATION_GUIDE.txt - Implementation specifics
3. scheduler.c - Well-commented source code
4. Sample outputs - See actual behavior patterns


================================================================================
Project Ready for Submission
Compiled and tested June 7, 2026
================================================================================
