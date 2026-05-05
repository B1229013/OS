# Report Template — OSP-Homework-B1229013.pdf

Constraints: **2 A4 pages max**, **12 pt font**, **PDF**. Filename must be exactly `OSP-Homework-B1229013.pdf`.

Paste the text below into Word, replace `<Your Name>` and the screenshot placeholder, then export → PDF.

---

## PAGE 1

**OSP Homework 1**
**B1229013**
**<Your Name>**

### Implement

After uC/OS-II is initialized in `main()` — `OSInit()`, `PC_DOSSaveReturn()`, `PC_VectSet(uCOS, OSCtxSw)` to install the context-switch handler, `OSTaskCreate` to register `TaskStart` at priority 0, then `OSStart()` — control passes to `TaskStart`, which is the only application task in this design.

`TaskStart` performs Step 1 through Step 4 sequentially: `PC_DispStr` writes the banner "This is OPS homework!" at row 0, and `OSTimeDlyHMSM(0, 0, 3, 0)` blocks for 3 seconds. The same pattern then prints the student ID `B1229013` at row 2 and waits 2 seconds.

Steps 5, 6, and 7 sit inside an infinite `for(;;)` loop. Step 5 is an inner for-loop counting from 13 down to 0; each iteration formats the number with `sprintf("%d ")`, prints it at row 5 with `PC_DispStr` at column `pos_x`, advances `pos_x` by 3 columns for two-digit numbers and 2 for single-digit numbers, then calls `OSTimeDly(200)` — 200 ticks at `OS_TICKS_PER_SEC = 200` equals one second. Step 6 overwrites row 5 with spaces and waits 3 seconds via `OSTimeDlyHMSM`. Step 7 is implicit because the outer `for(;;)` jumps back to Step 5. Pressing ESC is checked once per outer iteration and calls `PC_DOSReturn` to exit cleanly.

[ insert screenshot of TaskStart() function from VS Code or Notepad++ ]

---

## PAGE 2

### The problem you met, and how you solved it.

Two issues came up during implementation.

**(1) Two-digit overwrite.** My first version printed every number at the same x position, so printing "9" after "13" left a "3" character behind from the previous frame ("13" → overwrite with "9 " only clears the first two columns, leaving the third). I fixed it by tracking `pos_x` separately and stepping it forward 3 columns for two-digit values (10–13) and 2 columns for single-digit values (0–9). Each number now occupies its own slot and the row reads "13 12 11 10 9 8 7 6 5 4 3 2 1 0" cleanly. A second attempt was to use a fixed-width `%2d` format, but that left a stray space in front of single-digit numbers, so the variable-width approach matched the spec better.

**(2) Choosing between OSTimeDly and OSTimeDlyHMSM.** Both achieve a delay, but `OSTimeDly` takes raw OS ticks — and the actual wall-clock duration depends on `OS_TICKS_PER_SEC`, which is 200 on this BC45/x86 port. `OSTimeDlyHMSM` takes hours/minutes/seconds/milliseconds and is portable across configurations. I used `OSTimeDlyHMSM` for the spec's "3 second" and "2 second" pauses (Steps 2, 4, 6) so the code reads naturally, and `OSTimeDly(200)` inside the inner countdown loop (Step 5) so both APIs are exercised as the assignment requests.

A third subtle issue: the reference example I read started by spawning multiple worker tasks, one per number, with each task ending in `OSTaskDel(OS_PRIO_SELF)`. That design technically does not satisfy Step 7 ("Goto Step 5") because once every worker has self-deleted, no task remains to restart the sequence. A single-task solution with an outer `for(;;)` is simpler and meets the loop requirement directly.

### Reference

- Course slides — *An Real-Time OS: µC/OS-II Quick Overview* (Prof. Che-Wei Chang)
- µC/OS-II e-book, Chapter 1
- https://micrium.atlassian.net/wiki/spaces/osiidoc/
- https://www.csie.ntu.edu.tw/~ktw/rts/uCOSII-prn.pdf
