#include "verilated_dpi.h"
#include "VCPU.h"
#include "verilated_vcd_c.h"
#include <bits/stdc++.h>
#include "include/debug.h"
#include "include/common.h"
#include "include/disasm.h"
#include "memory/paddr.h"

using namespace std;

extern VCPU *dut;
extern uint64_t sim_time;
extern VerilatedVcdC *m_trace;

extern uint8_t pmem[];

void print_itrace();
void difftest_step();

// Lab2 HINT: instruction log struct for instruction trace
struct inst_log
{
    word_t pc;
    word_t inst;
};
struct inst_log inst_log_queue[17];
int front = 0;
int rear = 0;
uint32_t *cpu_mstatus = NULL, *cpu_mtvec = NULL, *cpu_mepc = NULL, *cpu_mcause = NULL;
// load the state of your simulated cpu into sim_cpu
void set_state()
{
    sim_cpu.pc = dut->pc_cur;
    memcpy(&sim_cpu.gpr[0], cpu_gpr, 4 * 32);
    // Lab4 TODO: set the state of csr to sim_cpu
    memcpy(&sim_cpu.csr.mcause, cpu_mcause, 4);
    memcpy(&sim_cpu.csr.mepc, cpu_mepc, 4);
    memcpy(&sim_cpu.csr.mstatus, cpu_mstatus, 4);
    memcpy(&sim_cpu.csr.mtvec, cpu_mtvec, 4);
}

// num of executed instruction
uint64_t g_nr_guest_inst = 0;

// simulate a single cycle
void single_cycle()
{
    // Lab2 TODO: implement the single cycle function of your cpu

    m_trace->dump(sim_time++);
    dut->clk = 1;
    dut->eval();
    m_trace->dump(sim_time++);
    if (dut->commit_wb == 1)
        set_state();
    dut->clk = 0;
    dut->eval();
    m_trace->dump(sim_time++);
}

// simulate a reset
void reset(int n)
{
    dut->clk = 0;
    dut->rstn = 0;
    dut->eval();
    while (n-- > 0)
    {
        single_cycle();
    }
    dut->rstn = 1;
    dut->eval();
}

// check if the program should end
inline bool test_break()
{
    return dut->inst == 0x00100073U;
}

static void statistic()
{
    Log("total guest instructions = %ld", g_nr_guest_inst);
}

void device_update();
// init the running state of our simulator
SimState sim_state = {.state = SIM_STOP};

void add_to_queue()
{
    if (dut->inst == 0x00000013)
        return;
    if (dut->inst == 0x00000000)
        return;
    if (dut->inst == inst_log_queue[(rear - 1) % 17].inst)
        return;
    if ((rear + 1) % 17 == front) // 目前队满
    {
        front = (front + 1) % 17; // 移动队头
    }
    inst_log_queue[rear].inst = dut->inst;
    inst_log_queue[rear].pc = dut->pc_cur;
    rear = (rear + 1) % 17;
}
void print_queue()
{
    // printf("%d,%d\n", front, rear);
    int i = front;
    int k = 1;
    while (i != rear)
    {
        printf("记录%d:PC:0x%x\tinst:0x%08x\t\t", k, inst_log_queue[i].pc, inst_log_queue[i].inst);
        word_t Word = inst_log_queue[i].inst;
        uint8_t bytes[5] = {0};
        bytes[0] = (Word & 0xFF);
        bytes[1] = ((Word >> 8) & 0xFF);
        bytes[2] = ((Word >> 16) & 0xFF);
        bytes[3] = ((Word >> 24) & 0xFF);
        char inst[256] = {0};
        disassemble(inst, 256, inst_log_queue[i].pc, bytes, 4);
        printf("%s\n", inst);
        i = (i + 1) % 17;
        k++;
    }
}
// execute n instructions
void cpu_exec(unsigned int n)
{
    bool f = n > 1;
    switch (sim_state.state)
    {
    case SIM_END:
    case SIM_ABORT:
    case SIM_QUIT:
    {
        printf("Program execution has ended. To restart the program, exit NPC and run again.\n");
        print_queue();
        return;
    }
    default:
        sim_state.state = SIM_RUNNING;
    }
    // Lab2 TODO: implement instruction trace for your cpu
    bool npc_cpu_uncache_pre = 0;
    while (n--)
    {
        // execute single instruction
        if (test_break())
        {
            // set the end state
            sim_state.halt_pc = dut->pc_cur;
            sim_state.halt_ret = cpu_gpr[10];
            sim_state.state = SIM_END;
            print_queue();
            break;
        }

        if (dut->commit_wb)
        {
            if (npc_cpu_uncache_pre)
            {
                difftest_sync();
            }
            // Lab3 TODO: use difftest_step function here to execute difftest
            difftest_step();

            npc_cpu_uncache_pre = dut->uncache_read_wb;
        }
        g_nr_guest_inst++;
        // your cpu step a cycle
        add_to_queue();
        single_cycle();
        // print_queue();

#ifdef DEVICE
        device_update();
#endif
        if (sim_state.state != SIM_RUNNING)
            break;
    }
    // if (f) // si指令输出队列
    print_queue();
    switch (sim_state.state)
    {
    case SIM_RUNNING:
        sim_state.state = SIM_STOP;
        break;
    case SIM_END:
    case SIM_ABORT:
        Log("sim: %s at pc = " FMT_WORD,
            (sim_state.state == SIM_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : (sim_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
            sim_state.halt_pc);
        // fall through
    case SIM_QUIT:
        statistic();
    }
}

static const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

// map the name of reg to its value
word_t isa_reg_str2val(const char *s, bool *success)
{
    ;
    if (!strcmp(s, "pc"))
    {
        *success = true;
        return dut->pc_cur;
    }
    for (int i = 0; i < 32; i++)
    {
        if (!strcmp(s, regs[i]))
        {
            *success = true;
            return cpu_gpr[i];
        }
    }
    *success = false;
    return 0;
}

// set cpu_gpr point to your cpu's gpr
extern "C" void set_gpr_ptr(const svOpenArrayHandle r)
{
    cpu_gpr = (uint32_t *)(((VerilatedDpiOpenVar *)r)->datap());
}
// set the pointers pint to you cpu's csr
extern "C" void set_csr_ptr(const svOpenArrayHandle mstatus, const svOpenArrayHandle mtvec, const svOpenArrayHandle mepc, const svOpenArrayHandle mcause)
{
    cpu_mstatus = (uint32_t *)(((VerilatedDpiOpenVar *)mstatus)->datap());
    cpu_mtvec = (uint32_t *)(((VerilatedDpiOpenVar *)mtvec)->datap());
    cpu_mepc = (uint32_t *)(((VerilatedDpiOpenVar *)mepc)->datap());
    cpu_mcause = (uint32_t *)(((VerilatedDpiOpenVar *)mcause)->datap());
}

void isa_reg_display()
{
    for (int i = 0; i < 32; i += 2)
    {
        printf("gpr[%d](%s) = 0x%x  0x%x\t\t", i, regs[i], cpu_gpr[i], sim_cpu.gpr[i]);
        printf("gpr[%d](%s) = 0x%x  0x%x\n", i + 1, regs[i + 1], cpu_gpr[i + 1], sim_cpu.gpr[i + 1]);
    }
}

void print_itrace()
{
    // Lab2 HINT: you can implement this function to help you print the instruction trace
}
