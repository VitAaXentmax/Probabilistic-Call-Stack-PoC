# 🧬 Probabilistic Call Stack PoC

> A proof-of-concept to demonstrate randomized execution paths and their impact on call stack signatures — ideal for EDR testing, behavior-based detection research, and evasion analysis.

## 📌 About

This project showcases how a payload can be executed through **multiple unique call paths**, each producing a **distinct call stack**, even though the final behavior remains identical.

The goal is to challenge **signature-based and stack-trace-based detection engines** used in Endpoint Detection and Response (EDR) solutions.

## ⚙️ Features

- 🧠 **13 randomized wrapper paths** (nested, recursive, indirect, pointer chain, deep, branching, etc.)
- 🔍 **Stack trace capture** using `RtlCaptureStackBackTrace()` and `dbghelp`
- 💬 **Benign payload**: MessageBox to simulate execution
- 📈 **Per-execution statistics** to track path usage
- ⏱️ **Auxiliary noise functions** to alter call stack structure (heap, time, system info, env vars)
- 🧪 **Demonstration mode** with configurable number of runs

## 🧪 Use Cases

- Testing EDR solutions against obfuscated execution paths
- Security research on behavior-based detection
- Teaching control-flow obfuscation techniques
- Red Team payload delivery testing
- Malware analysis and sandbox evasion concepts

## 🚀 How It Works

Each run randomly selects one of 13 wrapper functions. These wrappers contain different styles of call stack construction:

| Path | Description |
|------|-------------|
| A | Direct execution |
| B | Nested calls |
| C | Deep nesting |
| D | Function pointer indirection |
| E | Recursion |
| F | Heavy auxiliary calls |
| G | Virtual memory allocation |
| H | 5-level tower |
| I | 6-level deep stack with system calls |
| J | Function pointer chain |
| K | Mixed recursion + nesting |
| L | 7-level "staircase" with system + env queries |
| M | Branching with 4 random paths |

All paths eventually converge on `execute_payload()` — a benign MessageBox.

## 🧰 Compilation

### 🪟 MSVC (Visual Studio Developer Command Prompt)
```bash
cl /EHsc /O2 probabilistic_callstack.cpp /link user32.lib dbghelp.lib
````

### 🐧 MinGW

```bash
g++ -O2 probabilistic_callstack.cpp -o probabilistic_callstack.exe -luser32 -ldbghelp
```

> ⚠️ Requires Windows. Tested on Windows 10/11 x64.

## 🖥️ Example Output

```
EXECUTION #2
Selected: Path L (Staircase - 7 levels)

[STACK TRACE] Before wrapper execution (depth: 12 frames)
  [00] 0x00007FF6AC102390 main
  [01] 0x00007FF6AC101F40 run_demonstration
  [02] 0x00007FF6AC101A60 execute_random_path
  [03] 0x00007FF6AC100C10 wrapper_path_L_staircase
  ...
[PAYLOAD] Executing final payload (execution #2)
```

## 📊 Stats Tracking

After all executions, you will see statistics like:

```
PATH USAGE STATISTICS
  Path A (Direct): 1 times
  Path B (Nested): 0 times
  Path C (Deep): 2 times
  ...
```

## 📎 Notes

* No malicious behavior is implemented. This is a safe educational PoC.
* Call stack depth varies across paths: from 2 up to 10+ frames.
* Ideal for studying how **EDRs correlate behavior using stack patterns**.

## ✅ License

MIT License

---

**Author:** Joas A Santos
**Purpose:** For educational and research use only.
