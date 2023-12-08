#include <debug.h>
#include <proc.h>
#include <stdio.h>
#define ECALL_FROM_M 0xb
#define SYSCALL_YIELD 0xffffffff
#define BAD_PC 0xc
#define BAD_INST 0x2
#define PC_FAULT 0x1
#define EBREAK 0x3
static const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
void syscall_handle(Context *c);
void exp_handle(Context *c);
static Context *__event_handle(Event e, Context *c);

Context *__irq_handle(Context *c)
{
    Event ev = {0};
    // printf("0x%x\n", c->mtval);
    switch (c->mcause)
    {
        // Lab7 TODO: implement EVENT_SYSCALL and EVENT_YIELD
    case ECALL_FROM_M:
    {
        switch (c->gpr[17])
        {
        case -1:
        {
            ev.event = EVENT_YIELD;
            break;
        }
        default:
        {
            ev.event = EVENT_SYSCALL;
            break;
        }
        }
        break;
    }
    default:
    {
        ev.event = EVENT_ERROR;
        break;
    }
    }
    c = __event_handle(ev, c);
    assert(c != NULL);
    return c;
}

static Context *__event_handle(Event e, Context *c)
{
    switch (e.event)
    {
    case EVENT_YIELD:
        c->mepc += 4;
        break;
    case EVENT_SYSCALL:
        syscall_handle(c);
        c->mepc += 4;
        break;
    default:
        exp_handle(c);
        // panic("Unhandled event ID = %d", e.event);
    }

    return c;
}

extern void __trap_vector(void);

void exp_handle(Context *c)
{
    switch (c->mcause)
    {
    case BAD_PC:
    {
        Log("wrong pc!");
        c->mepc = c->mtval & 0xfffffffc;
        break;
    }
    case BAD_INST:
    {
        Log("wrong instruction!");
        c->mepc = c->mepc + 4;
        break;
    }
    case PC_FAULT:
    {
        Log("pc fault!");
        halt(1);
        break;
    }
    case EBREAK:
    {
        Log("ebreak!");
        for (int i = 0; i < 32; i += 2)
        {
            Log("gpr[%d](%s) = 0x%x\t\t\t\tgpr[%d](%s)", i, regs[i], c->gpr[i],i+1,regs[i+1],c->gpr[i+1]);
        }
        Log("mepc = 0x%x\t\t", c->mepc);
        Log("mstatus = 0x%x", c->mstatus);
        Log("mcause = 0x%x\t\t", c->mcause);
        Log("mtval = 0x%x", c->mtval);
        c->mepc += 4;
        break;
    }
    default:
        break;
    }
}
void yield()
{
    asm volatile("li a7, -1; ecall");
}

void init_irq(void)
{
    Log("Initializing interrupt/exception handler...");
    asm volatile("csrw mtvec, %0" : : "r"(__trap_vector));
}
