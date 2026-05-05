# OSP Homework 1 — Step-by-Step Guide

**Student ID:** B1229013
**Last two digits (XY):** 13 → countdown is 13, 12, 11, …, 0
**Deadline:** 23:00 2026/05/06
**Deliverables:**
1. Modified `TEST.C` (your code)
2. PDF report: `OSP-Homework-B1229013.pdf` (2 A4 pages, 12 pt font)
3. Live demo to TA

---

## Phase 1 — Set up the build environment

Borland C++ 4.5 + TASM only run cleanly on **32-bit Windows XP**, so do everything inside a VirtualBox VM.

1. Install **VirtualBox** on your Windows 10 host.
2. Create a VM with **Windows XP 32-bit**. Install Guest Additions.
3. Inside the VM, install **Borland C++ 4.5** by running its `INSTALL.EXE`. Default install path: `C:\BC45`.
4. Add `;C:\BC45\BIN` to the **system PATH**:
   Control Panel → System → Advanced → Environment Variables → edit `Path`. **No spaces.**
5. Install **Turbo Assembler**: unzip the TASM package, copy `tasm\BIN\TASM.EXE` into `C:\BC45\BIN`. (BC45 ships without an assembler; the µC/OS-II port has `.ASM` files that need it.)
6. Place the µC/OS-II source package directly at `C:\SOFTWARE\` so the path `C:\SOFTWARE\uCOS-II\...` exists. The build script assumes this.

**Sanity check:** open `cmd`, type `tasm` and `bcc`. Both should print a banner, not "not recognized".

---

## Phase 2 — Run the stock example first

Before changing anything, confirm the unmodified example builds and runs.

1. Run `C:\SOFTWARE\uCOS-II\EX1_x86L\BC45\TEST\TEST.EXE`. You should see the multitasking demo with random digits filling the screen. Press **ESC** to exit.
2. Rename or delete that `TEST.EXE` so you can tell when your build worked.
3. In `cmd`, `cd` into `C:\SOFTWARE\uCOS-II\EX1_x86L\BC45\TEST\`, run `MAKETEST.BAT`. A fresh `TEST.EXE` should appear.
4. If it doesn't: re-check PATH, TASM location, and that SOFTWARE is at the root of `C:\`.

The `MAKETEST 1.bat` you have is a slightly nicer variant of the default — it timestamps backups of `TEST.C` into `..\OLD_TEST\` and pipes compiler output to `..\LOG\COMPILE_LOG.TXT`. Drop it in place of the default to get those niceties (rename it to `MAKETEST.BAT`).

---

## Phase 3 — Understand the program shape

The application source you'll edit is `C:\SOFTWARE\uCOS-II\EX1_x86L\BC45\SOURCE\TEST.C`. Control flow:

```
main()                      // initializes uC/OS-II, creates TaskStart, calls OSStart
  └── TaskStart()           // first task to run; does all the homework steps
```

You will only edit **TEST.C** — that's it.

### Key API quick-reference

| Function | Purpose |
|---|---|
| `PC_DispStr(x, y, "text", color)` | Print string at column `x`, row `y` |
| `PC_DispChar(x, y, ch, color)` | Print one character |
| `OSTimeDlyHMSM(h, m, s, ms)` | Sleep H hours, M min, S sec, MS ms |
| `OSTimeDly(ticks)` | Sleep N OS ticks (200 ticks ≈ 1 s on this port) |
| `OSTaskCreate(fn, arg, top_of_stack, prio)` | Spawn a task (0 = highest priority) |
| `OSInit()` | Initialize µC/OS-II — must be called before anything else |
| `OSStart()` | Start the kernel; never returns to `main` |
| `PC_DOSReturn()` | Clean exit back to DOS |

---

## Phase 4 — Replace TEST.C with the version below

Open `C:\SOFTWARE\uCOS-II\EX1_x86L\BC45\SOURCE\TEST.C` inside the VM and replace the entire contents with the code in `TEST.C` (next to this file). Save.

### Why the design choices

- **Single TaskStart task.** The spec only requires `OSTimeDly` and `OSTimeDlyHMSM`. One task cleanly implements the linear flow 1→2→3→4→5→6→7→5… The reference student used 10 worker tasks but their version doesn't actually loop (Step 7 is missing). This version satisfies the full spec.
- **`sprintf("%d ")` + variable `pos_x`.** Naturally handles "13 12 11 10 9 8 …" with no leftover digits when the width changes from 2 to 1. Avoids the reference student's `'0' + 10 == ':'` trap.
- **Both delay APIs used.** `OSTimeDlyHMSM` for the 3 s and 2 s pauses (Steps 2, 4, 6); `OSTimeDly(200)` for the 1 s pause inside Step 5. Demonstrates both as the spec requests.

---

## Phase 5 — Compile and run

```
cd C:\SOFTWARE\uCOS-II\EX1_x86L\BC45\TEST
MAKETEST.BAT
TEST.EXE
```

Expected screen:
```
This is OPS homework!

B1229013


13 12 11 10 9 8 7 6 5 4 3 2 1 0
```
…then row 5 clears, 3-second pause, sequence replays. **ESC** to exit.

### Common compile failures

| Symptom | Cause |
|---|---|
| `TASM not found` | TASM.EXE wasn't copied into `C:\BC45\BIN` |
| `cannot find ..\TEST\TEST.MAK` | SOFTWARE is not at the root of `C:\` |
| `bcc command not found` | PATH is wrong (missing `;C:\BC45\BIN`) |
| Compile log full of weird errors | TEST.C wasn't saved, or saved with wrong line endings |

If something fails, read `..\LOG\COMPILE_LOG.TXT` first (your timestamped variant writes there).

---

## Phase 6 — Write the 2-page PDF report

Filename: `OSP-Homework-B1229013.pdf`. Constraints: 2 A4 pages max, 12 pt font, PDF only. Upload to E-learning before **23:00 2026/05/06**.

See `report-template.md` next to this file for the full text you can paste into Word and export to PDF.

Sections (matches grading expectations):
1. **Title block** — `OSP Homework 1` / `B1229013` / your name
2. **Implement** — short paragraph + screenshot of `TaskStart()`
3. **The problem you met, and how you solved it** — 1–2 paragraphs (use the two-digit overwrite + delay-API choice issues)
4. **Reference** — URLs you actually used

---

## Phase 7 — Demo (40% of the grade)

When you sit with the TA:

1. Show `MAKETEST.BAT` compiling cleanly.
2. Run `TEST.EXE` and walk through Steps 1–7 as they appear on screen.
3. Press ESC; program returns to DOS cleanly.
4. Be ready to answer:
   - What does `OSInit` do? (initializes ready list, priority table, TCB pool, idle + stat tasks)
   - Why does `OSTaskCreate` take the *top* of the stack? (x86 stack grows downward)
   - Why is priority 0 the highest? (lower number = higher priority by µC/OS-II convention; ≤ 7 reserved)
   - Difference between `OSTimeDly` and `OSTimeDlyHMSM`? (raw ticks vs portable H/M/S/MS)
   - What would happen without `OSStatInit`? (CPU usage stats wouldn't run, but program still works)

---

## Files in this folder

| File | Purpose |
|---|---|
| `instructions.md` | This guide |
| `TEST.C` | Ready-to-use homework C source — paste into `C:\SOFTWARE\uCOS-II\EX1_x86L\BC45\SOURCE\TEST.C` inside the VM |
| `report-template.md` | Text content for the 2-page PDF report |

The original course materials (PDF + reference DOCX + MAKETEST 1.bat) are still in `C:\Users\user\Downloads\`.

---

## Resume notes

- Currently at Phase: **(update this when you stop)** ___
- Last thing tried: ___
- Next thing to do: ___
