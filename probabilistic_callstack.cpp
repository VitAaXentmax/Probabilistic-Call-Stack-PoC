/**
 * Probabilistic Call Stack PoC
 *
 * Purpose: Demonstrate how randomized execution paths create varying call stacks
 * Use Case: EDR testing and detection pattern analysis
 *
 * This PoC creates multiple wrapper functions that eventually call the same
 * payload. Each execution randomly selects a path, resulting in different
 * call stack signatures while maintaining identical final behavior.
 *
 * Compile with MSVC: cl /EHsc /O2 probabilistic_callstack.cpp /link user32.lib
 * Compile with MinGW: g++ -O2 probabilistic_callstack.cpp -o probabilistic_callstack.exe -luser32
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "user32.lib")

// Configuration
#define NUM_WRAPPERS 13
#define MAX_STACK_DEPTH 64
#define ENABLE_STACK_TRACE 1

// Forward declarations
typedef void (*PayloadFunc)();
void execute_payload();
void capture_and_print_stack(const char* context);

// Global execution counter for demonstration
static int g_execution_id = 0;

//=============================================================================
// PAYLOAD SECTION
// The final function that all paths lead to
//=============================================================================

void execute_payload() {
    printf("\n[PAYLOAD] Executing final payload (execution #%d)\n", g_execution_id);

    // Benign payload: Display a message box
    MessageBoxA(NULL,
                "Payload executed successfully!\n\nCall stack was randomized.",
                "Probabilistic Call Stack PoC",
                MB_OK | MB_ICONINFORMATION);

    printf("[PAYLOAD] Payload completed\n");
}

//=============================================================================
// STACK TRACE CAPTURE
// Uses RtlCaptureStackBackTrace to display current call stack
//=============================================================================

void capture_and_print_stack(const char* context) {
#if ENABLE_STACK_TRACE
    void* stack[MAX_STACK_DEPTH];
    USHORT frames;

    frames = RtlCaptureStackBackTrace(0, MAX_STACK_DEPTH, stack, NULL);

    printf("\n[STACK TRACE] %s (depth: %d frames)\n", context, frames);
    printf("----------------------------------------\n");

    // Initialize symbol handler for address resolution
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    char symbol_buffer[sizeof(SYMBOL_INFO) + 256];
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbol_buffer;
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (USHORT i = 0; i < frames && i < 15; i++) {
        DWORD64 address = (DWORD64)stack[i];

        if (SymFromAddr(process, address, NULL, symbol)) {
            printf("  [%2d] 0x%016llX %s\n", i, address, symbol->Name);
        } else {
            printf("  [%2d] 0x%016llX <unknown>\n", i, address);
        }
    }

    if (frames > 15) {
        printf("  ... (%d more frames)\n", frames - 15);
    }

    printf("----------------------------------------\n\n");

    SymCleanup(process);
#endif
}

//=============================================================================
// AUXILIARY FUNCTIONS
// Used within wrappers to add stack depth and variation
//=============================================================================

// Simple delay to add timing variation
void aux_small_delay() {
    Sleep(rand() % 10 + 1);
}

// Get system time - adds API call to stack trace
void aux_get_time() {
    SYSTEMTIME st;
    GetSystemTime(&st);
    printf("  [AUX] System time: %02d:%02d:%02d.%03d\n",
           st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

// Heap allocation - adds memory operation to trace
void aux_heap_operation() {
    HANDLE heap = GetProcessHeap();
    void* mem = HeapAlloc(heap, HEAP_ZERO_MEMORY, 64);
    if (mem) {
        printf("  [AUX] Heap allocated at: 0x%p\n", mem);
        HeapFree(heap, 0, mem);
    }
}

// Query performance counter - adds timing API
void aux_query_perf() {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    printf("  [AUX] Performance counter: %lld (freq: %lld)\n",
           counter.QuadPart, freq.QuadPart);
}

// Get thread context info
void aux_thread_info() {
    DWORD tid = GetCurrentThreadId();
    DWORD pid = GetCurrentProcessId();
    printf("  [AUX] PID: %lu, TID: %lu\n", pid, tid);
}

//=============================================================================
// WRAPPER FUNCTIONS
// Each provides a different path to the payload with unique stack signature
//=============================================================================

// Path A: Direct path with minimal overhead
void wrapper_path_A_direct() {
    printf("[PATH A] Direct execution path\n");
    aux_thread_info();
    execute_payload();
}

// Path B: Nested call through intermediate function
void wrapper_path_B_inner() {
    printf("  [PATH B] Inner wrapper\n");
    aux_get_time();
    execute_payload();
}

void wrapper_path_B_nested() {
    printf("[PATH B] Nested execution path\n");
    aux_small_delay();
    wrapper_path_B_inner();
}

// Path C: Deep nesting with multiple layers
void wrapper_path_C_level3() {
    printf("    [PATH C] Level 3\n");
    execute_payload();
}

void wrapper_path_C_level2() {
    printf("  [PATH C] Level 2\n");
    aux_query_perf();
    wrapper_path_C_level3();
}

void wrapper_path_C_deep() {
    printf("[PATH C] Deep nested path\n");
    aux_heap_operation();
    wrapper_path_C_level2();
}

// Path D: Uses function pointer indirection
void wrapper_path_D_indirect() {
    printf("[PATH D] Indirect execution via function pointer\n");
    aux_get_time();

    // Store payload address in function pointer
    PayloadFunc ptr = execute_payload;
    printf("  [PATH D] Calling through pointer: 0x%p\n", (void*)ptr);
    ptr();
}

// Path E: Recursive path with countdown
void wrapper_path_E_recursive(int depth) {
    printf("  [PATH E] Recursion depth: %d\n", depth);

    if (depth <= 0) {
        aux_thread_info();
        execute_payload();
    } else {
        aux_small_delay();
        wrapper_path_E_recursive(depth - 1);
    }
}

void wrapper_path_E_entry() {
    printf("[PATH E] Recursive execution path\n");
    int recursion_depth = rand() % 3 + 1;  // 1-3 levels
    wrapper_path_E_recursive(recursion_depth);
}

// Path F: Multiple auxiliary calls before payload
void wrapper_path_F_heavy() {
    printf("[PATH F] Heavy auxiliary path\n");
    aux_thread_info();
    aux_get_time();
    aux_heap_operation();
    aux_query_perf();
    aux_small_delay();
    execute_payload();
}

// Path G: VirtualAlloc-based dummy execution
void wrapper_path_G_virtual() {
    printf("[PATH G] VirtualAlloc memory path\n");

    // Allocate executable memory
    void* mem = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                             PAGE_EXECUTE_READWRITE);

    if (mem) {
        printf("  [PATH G] Allocated executable memory at: 0x%p\n", mem);

        // Store the address (simulating indirect call setup)
        volatile DWORD_PTR addr = (DWORD_PTR)mem;
        printf("  [PATH G] Memory region active\n");

        // Free before calling actual payload
        VirtualFree(mem, 0, MEM_RELEASE);
        printf("  [PATH G] Memory released, calling payload\n");
    }

    aux_heap_operation();
    execute_payload();
}

//=============================================================================
// DEEP NESTING WRAPPER FUNCTIONS
// Paths with 5+ levels of call depth
//=============================================================================

// Path H: 5-level tower with alternating aux calls
void wrapper_path_H_level5() {
    printf("          [PATH H] Level 5 - Final\n");
    aux_thread_info();
    execute_payload();
}

void wrapper_path_H_level4() {
    printf("        [PATH H] Level 4\n");
    aux_query_perf();
    wrapper_path_H_level5();
}

void wrapper_path_H_level3() {
    printf("      [PATH H] Level 3\n");
    aux_heap_operation();
    wrapper_path_H_level4();
}

void wrapper_path_H_level2() {
    printf("    [PATH H] Level 2\n");
    aux_get_time();
    wrapper_path_H_level3();
}

void wrapper_path_H_level1() {
    printf("  [PATH H] Level 1\n");
    aux_small_delay();
    wrapper_path_H_level2();
}

void wrapper_path_H_tower() {
    printf("[PATH H] 5-Level Tower path\n");
    wrapper_path_H_level1();
}

// Path I: 6-level deep with memory operations at each level
void wrapper_path_I_level6() {
    printf("            [PATH I] Level 6 - Terminus\n");
    execute_payload();
}

void wrapper_path_I_level5() {
    printf("          [PATH I] Level 5\n");
    HANDLE heap = GetProcessHeap();
    void* m = HeapAlloc(heap, 0, 32);
    wrapper_path_I_level6();
    if (m) HeapFree(heap, 0, m);
}

void wrapper_path_I_level4() {
    printf("        [PATH I] Level 4\n");
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    wrapper_path_I_level5();
}

void wrapper_path_I_level3() {
    printf("      [PATH I] Level 3\n");
    SYSTEMTIME st;
    GetLocalTime(&st);
    wrapper_path_I_level4();
}

void wrapper_path_I_level2() {
    printf("    [PATH I] Level 2\n");
    DWORD tid = GetCurrentThreadId();
    (void)tid;
    wrapper_path_I_level3();
}

void wrapper_path_I_level1() {
    printf("  [PATH I] Level 1\n");
    Sleep(1);
    wrapper_path_I_level2();
}

void wrapper_path_I_deep6() {
    printf("[PATH I] 6-Level Deep path\n");
    wrapper_path_I_level1();
}

// Path J: Chain of function pointer calls (5 levels)
typedef void (*ChainFunc)();

void wrapper_path_J_final() {
    printf("          [PATH J] Chain end\n");
    aux_heap_operation();
    execute_payload();
}

void wrapper_path_J_link4() {
    printf("        [PATH J] Link 4\n");
    ChainFunc next = wrapper_path_J_final;
    aux_small_delay();
    next();
}

void wrapper_path_J_link3() {
    printf("      [PATH J] Link 3\n");
    ChainFunc next = wrapper_path_J_link4;
    aux_query_perf();
    next();
}

void wrapper_path_J_link2() {
    printf("    [PATH J] Link 2\n");
    ChainFunc next = wrapper_path_J_link3;
    aux_get_time();
    next();
}

void wrapper_path_J_link1() {
    printf("  [PATH J] Link 1\n");
    ChainFunc next = wrapper_path_J_link2;
    aux_thread_info();
    next();
}

void wrapper_path_J_chain() {
    printf("[PATH J] Function Pointer Chain (5 links)\n");
    ChainFunc start = wrapper_path_J_link1;
    start();
}

// Path K: Mixed recursion with nesting (variable depth 4-7)
void wrapper_path_K_nested_inner() {
    printf("        [PATH K] Nested inner\n");
    aux_get_time();
    execute_payload();
}

void wrapper_path_K_nested_outer() {
    printf("      [PATH K] Nested outer\n");
    aux_heap_operation();
    wrapper_path_K_nested_inner();
}

void wrapper_path_K_recursive(int depth) {
    printf("    [PATH K] Recursive level: %d\n", depth);
    aux_small_delay();

    if (depth <= 0) {
        wrapper_path_K_nested_outer();
    } else {
        wrapper_path_K_recursive(depth - 1);
    }
}

void wrapper_path_K_mixed() {
    printf("[PATH K] Mixed Recursion + Nesting path\n");
    int depth = rand() % 4 + 1;  // 1-4 recursion levels + 2 nested = 4-7 total
    printf("  [PATH K] Selected recursion depth: %d\n", depth);
    wrapper_path_K_recursive(depth);
}

// Path L: 7-level staircase with registry/environment queries
void wrapper_path_L_level7() {
    printf("              [PATH L] Level 7 - Summit\n");
    execute_payload();
}

void wrapper_path_L_level6() {
    printf("            [PATH L] Level 6\n");
    // Query environment variable
    char buf[64];
    GetEnvironmentVariableA("PATH", buf, 10);
    wrapper_path_L_level7();
}

void wrapper_path_L_level5() {
    printf("          [PATH L] Level 5\n");
    DWORD size = MAX_PATH;
    char compname[MAX_PATH];
    GetComputerNameA(compname, &size);
    wrapper_path_L_level6();
}

void wrapper_path_L_level4() {
    printf("        [PATH L] Level 4\n");
    MEMORYSTATUSEX memstat;
    memstat.dwLength = sizeof(memstat);
    GlobalMemoryStatusEx(&memstat);
    wrapper_path_L_level5();
}

void wrapper_path_L_level3() {
    printf("      [PATH L] Level 3\n");
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    wrapper_path_L_level4();
}

void wrapper_path_L_level2() {
    printf("    [PATH L] Level 2\n");
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    wrapper_path_L_level3();
}

void wrapper_path_L_level1() {
    printf("  [PATH L] Level 1\n");
    aux_thread_info();
    wrapper_path_L_level2();
}

void wrapper_path_L_staircase() {
    printf("[PATH L] 7-Level Staircase path\n");
    wrapper_path_L_level1();
}

// Path M: Branching deep path (random sub-paths within deep nesting)
void wrapper_path_M_terminus_alpha() {
    printf("          [PATH M] Terminus Alpha\n");
    aux_get_time();
    execute_payload();
}

void wrapper_path_M_terminus_beta() {
    printf("          [PATH M] Terminus Beta\n");
    aux_query_perf();
    execute_payload();
}

void wrapper_path_M_branch_level4() {
    printf("        [PATH M] Level 4 - Branch point\n");
    if (rand() % 2) {
        printf("        [PATH M] Taking Alpha branch\n");
        wrapper_path_M_terminus_alpha();
    } else {
        printf("        [PATH M] Taking Beta branch\n");
        wrapper_path_M_terminus_beta();
    }
}

void wrapper_path_M_level3() {
    printf("      [PATH M] Level 3\n");
    aux_heap_operation();
    wrapper_path_M_branch_level4();
}

void wrapper_path_M_level2_left() {
    printf("    [PATH M] Level 2 - Left\n");
    aux_small_delay();
    wrapper_path_M_level3();
}

void wrapper_path_M_level2_right() {
    printf("    [PATH M] Level 2 - Right\n");
    aux_thread_info();
    wrapper_path_M_level3();
}

void wrapper_path_M_level1() {
    printf("  [PATH M] Level 1 - Initial branch\n");
    if (rand() % 2) {
        printf("  [PATH M] Going left\n");
        wrapper_path_M_level2_left();
    } else {
        printf("  [PATH M] Going right\n");
        wrapper_path_M_level2_right();
    }
}

void wrapper_path_M_branching() {
    printf("[PATH M] Branching Deep path (5 levels, 4 possible routes)\n");
    wrapper_path_M_level1();
}

//=============================================================================
// PATH SELECTION AND EXECUTION
//=============================================================================

// Array of wrapper function pointers
typedef void (*WrapperFunc)();

WrapperFunc g_wrappers[NUM_WRAPPERS] = {
    wrapper_path_A_direct,
    wrapper_path_B_nested,
    wrapper_path_C_deep,
    wrapper_path_D_indirect,
    wrapper_path_E_entry,
    wrapper_path_F_heavy,
    wrapper_path_G_virtual,
    wrapper_path_H_tower,
    wrapper_path_I_deep6,
    wrapper_path_J_chain,
    wrapper_path_K_mixed,
    wrapper_path_L_staircase,
    wrapper_path_M_branching
};

const char* g_wrapper_names[NUM_WRAPPERS] = {
    "Path A (Direct)",
    "Path B (Nested - 2 levels)",
    "Path C (Deep - 3 levels)",
    "Path D (Indirect)",
    "Path E (Recursive - 1-3 levels)",
    "Path F (Heavy Aux)",
    "Path G (VirtualAlloc)",
    "Path H (Tower - 5 levels)",
    "Path I (Deep - 6 levels)",
    "Path J (Ptr Chain - 5 links)",
    "Path K (Mixed - 4-7 levels)",
    "Path L (Staircase - 7 levels)",
    "Path M (Branching - 5 levels)"
};

// Select and execute a random path
void execute_random_path() {
    int selected = rand() % NUM_WRAPPERS;

    printf("\n========================================\n");
    printf("EXECUTION #%d\n", g_execution_id);
    printf("Selected: %s\n", g_wrapper_names[selected]);
    printf("========================================\n\n");

    // Capture stack before execution
    capture_and_print_stack("Before wrapper execution");

    // Execute the selected wrapper
    g_wrappers[selected]();

    // Capture stack after (will show payload's stack)
    // Note: Stack will have unwound at this point
}

//=============================================================================
// DEMONSTRATION MODE
// Runs multiple executions to show stack variation
//=============================================================================

void run_demonstration(int num_runs) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     PROBABILISTIC CALL STACK DEMONSTRATION                   ║\n");
    printf("║     Running %d executions with randomized paths              ║\n", num_runs);
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    // Track which paths were used
    int path_usage[NUM_WRAPPERS] = {0};

    for (int i = 0; i < num_runs; i++) {
        g_execution_id = i + 1;

        // Record which path will be selected
        int preview = rand() % NUM_WRAPPERS;
        path_usage[preview]++;

        // Re-seed to get same selection
        srand((unsigned int)time(NULL) + i * 1000);

        execute_random_path();

        printf("\nPress Enter for next execution...\n");
        getchar();
    }

    // Print usage statistics
    printf("\n========================================\n");
    printf("PATH USAGE STATISTICS\n");
    printf("========================================\n");
    for (int i = 0; i < NUM_WRAPPERS; i++) {
        printf("  %s: %d times\n", g_wrapper_names[i], path_usage[i]);
    }
}

//=============================================================================
// MAIN ENTRY POINT
//=============================================================================

int main(int argc, char* argv[]) {
    // Seed random number generator
    srand((unsigned int)time(NULL));

    printf("Probabilistic Call Stack PoC\n");
    printf("For EDR testing and security research\n");
    printf("====================================\n\n");

    // Check command line arguments
    int num_runs = 3;  // Default

    if (argc > 1) {
        num_runs = atoi(argv[1]);
        if (num_runs < 1) num_runs = 1;
        if (num_runs > 10) num_runs = 10;
    }

    printf("Configuration:\n");
    printf("  - Number of wrappers: %d\n", NUM_WRAPPERS);
    printf("  - Planned executions: %d\n", num_runs);
    printf("  - Stack tracing: %s\n", ENABLE_STACK_TRACE ? "Enabled" : "Disabled");
    printf("\nEach execution will randomly select a different path,\n");
    printf("resulting in a unique call stack signature.\n");

    // Run the demonstration
    run_demonstration(num_runs);

    printf("\n========================================\n");
    printf("Demonstration complete.\n");
    printf("Each execution used a different call path,\n");
    printf("generating distinct stack signatures.\n");
    printf("========================================\n");

    return 0;
}
