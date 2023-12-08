#include <debug.h>
#include <assert.h>

void init_irq(void);
void init_fs(void);
void init_proc(void);
void init_mm(void);
bool bad_pc() // 指令地址未对齐
{
    asm volatile("auipc t0, 0");
    asm volatile("jalr t0, t0, 9");
    asm volatile("auipc t1, 0");
    asm volatile("csrr t0, mepc");
    unsigned int value1, value2;
    asm volatile("mv %0, t0" : "=r"(value1));
    asm volatile("mv %0, t1" : "=r"(value2));
    return value1 == value2;
}
bool bad_inst() // 非法指令
{
    asm volatile("auipc t0, 0");
    asm volatile("addi t0, t0, 12");
    asm volatile(".word 0x00000060"); // 插入一个非法指令
    asm volatile("csrr t1,mepc");
    unsigned int value1, value2;
    asm volatile("mv %0, t0" : "=r"(value1));
    asm volatile("mv %0, t1" : "=r"(value2));
    return value1 == value2;
}

bool ebreak()
{
    asm volatile("auipc t0, 0");
    asm volatile("addi t0, t0, 12");
    asm volatile("ebreak");
    asm volatile("csrr t1,mepc");
    unsigned int value1, value2;
    asm volatile("mv %0, t0" : "=r"(value1));
    asm volatile("mv %0, t1" : "=r"(value2));
    return value1 == value2;
}
void pc_fault()
{
    asm volatile("li t0, 0x82000000");
    asm volatile("jalr t0,0");
}

int main()
{
    extern const char logo[];
    printf("%s", logo);
    Log("Build time: %s, %s", __TIME__, __DATE__);
    Log("Start initialization...");

    init_mm();
    init_irq();
    init_fs();
    Log("Begin Tests");
    if (bad_pc())
        Log("Test bad_pc pass");
    else
        Log("Test bad_pc fail");
    if (bad_inst())
        Log("Test bad_inst pass");
    else
        Log("Test bad_inst fail");
    if (ebreak())
        Log("Test ebreak pass");
    else
        Log("Test ebreak fail");
    // Log("Test pc_fault begin");
    pc_fault();
    //   init_proc();

    Log("Finish initialization");
    yield();

    panic("Should not reach here");
}
