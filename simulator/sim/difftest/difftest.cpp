#include <dlfcn.h>
#include "common.h"
#include "debug.h"

enum
{
    DIFFTEST_TO_DUT,
    DIFFTEST_TO_REF
};
// the size of registers that should be checked in difftest, 32 gpr, 1 pc, 4 csr
#define DIFFTEST_REG_SIZE (sizeof(uint64_t) * (32 + 1 + 4))

// init fuction pointer with NULL, they will be assign when init
void (*difftest_regcpy)(void *dut, bool direction) = NULL;
void (*difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*difftest_exec)(uint64_t n) = NULL;
void (*difftest_raise_intr)(uint64_t NO) = NULL;

// should skip difftest
static bool is_skip_ref = false;
// the num of instruction that should be skipped
static int skip_dut_nr_inst = 0;

extern uint8_t pmem[];
static uint8_t ref_pmem[CONFIG_MSIZE];

// this is used to let ref skip instructions which
// can not produce consistent behavior with NEMU
void difftest_skip_ref()
{
    is_skip_ref = true;
    skip_dut_nr_inst = 0;
}

void difftest_skip_dut(int nr_ref, int nr_dut)
{
    skip_dut_nr_inst += nr_dut;

    while (nr_ref-- > 0)
    {
        difftest_exec(1);
    }
}

void init_difftest(char *ref_so_file, long img_size, int port)
{
    // ref_so_file is the nemu lib
    assert(ref_so_file != NULL);

    void *handle;
    Log("open dl: %s", ref_so_file);
    // open nemu lib, and link the difftest functions, they are implemented in nemu dir
    handle = dlopen(ref_so_file, RTLD_LAZY);
    assert(handle);

    difftest_memcpy = (void (*)(paddr_t, void *, size_t, bool))dlsym(handle, "difftest_memcpy");
    assert(difftest_memcpy);

    difftest_regcpy = (void (*)(void *, bool))dlsym(handle, "difftest_regcpy");
    assert(difftest_regcpy);

    difftest_exec = (void (*)(uint64_t))dlsym(handle, "difftest_exec");
    assert(difftest_exec);

    void (*difftest_init)(int) = (void (*)(int))dlsym(handle, "difftest_init");
    assert(difftest_init);

    Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));

    // init difftest
    difftest_init(port);
    // copy the memory, the registers, the pc to nemu, so our cpu and nemu can run with the same initial state
    difftest_memcpy(CONFIG_MBASE, guest_to_host(CONFIG_MBASE), CONFIG_MSIZE, DIFFTEST_TO_REF);
    sim_cpu.pc = CONFIG_MBASE;
    difftest_regcpy(&sim_cpu, DIFFTEST_TO_REF);
}

// copy our registers to nemu
void difftest_sync()
{
    difftest_regcpy(&sim_cpu, DIFFTEST_TO_REF);
}

// check the registers with nemu
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc)
{
    // check gpr
    // Lab3 TODO: implement the regfile check function, return false if any difference, and output some infomation of the difference
    int i = 0;
    // printf("checking\n");
    // printf("寄存器2状态不同,NEMU是%x,sim_cpu是%x\n", ref_r->gpr[2], sim_cpu.gpr[2]);
    while (i < 32)
    {
        // difftest_regcpy(&sim_cpu, DIFFTEST_TO_REF);
        if (ref_r->gpr[i] != cpu_gpr[i]) // 比较寄存器x[i]
        {
            // printf("寄存器%d状态相同,NEMU是%x,sim_cpu是%x\n", i - 1, ref_r->gpr[i - 1], sim_cpu.gpr[i - 1]);
            printf("寄存器%d状态不同,NEMU是%x,sim_cpu是%x\n", i, ref_r->gpr[i], sim_cpu.gpr[i]);
            // isa_reg_display();
            // int j = 0;
            // while (j < 32)
            // {
            //     printf("gpr[%d]=%x\n", j, sim_cpu.gpr[j]);
            //     j++;
            // }
            return false;
        }
        i++;
    }
    // check pc
    // Lab3 TODO: implement the pc check function, return false if any difference, and output some infomation of the difference
    if (pc != sim_cpu.pc)
    {
        printf("pc不一样,NEMU是%x,sim_cpu是%x\n", pc, sim_cpu.pc);
        // return false;
    }
    const char *csr_names[] = {"mepc", "mstatus", "mcause", "mtvec", "mtval"};
    // check csr
    // Lab4 TODO: (In Lab3, you can ignore this part.)implement the csr check function, return false if any difference, and output some infomation of the difference
    if (ref_r->csr.mepc != sim_cpu.csr.mepc)
    {
        printf("pc = 0x%x ", sim_cpu.pc);
        printf("%s不一样,NEMU是0x%x,sim_cpu是0x%x\n", csr_names[0], ref_r->csr.mepc, sim_cpu.csr.mepc);
        return false;
    }
    if (ref_r->csr.mstatus != sim_cpu.csr.mstatus)
    {
        printf("pc = 0x%x ", sim_cpu.pc);
        printf("%s不一样,NEMU是%x,sim_cpu是%x\n", csr_names[1], ref_r->csr.mstatus, sim_cpu.csr.mstatus);
        return false;
    }
    if (ref_r->csr.mcause != sim_cpu.csr.mcause)
    {
        printf("pc = 0x%x ", sim_cpu.pc);
        printf("%s不一样,NEMU是%x,sim_cpu是%x\n", csr_names[2], ref_r->csr.mcause, sim_cpu.csr.mcause);
        return false;
    }
    if (ref_r->csr.mtvec != sim_cpu.csr.mtvec)
    {
        printf("pc = 0x%x ", sim_cpu.pc);
        printf("%s不一样,NEMU是%x,sim_cpu是%x\n", csr_names[3], ref_r->csr.mtvec, sim_cpu.csr.mtvec);
        return false;
    }
    // if (ref_r->csr.mtval != sim_cpu.csr.mtval)
    // {
    //     printf("pc = 0x%x ", sim_cpu.pc);
    //     printf("%s不一样,NEMU是%x,sim_cpu是%x\n", csr_names[4], ref_r->csr.mtval, sim_cpu.csr.mtval);
    //     return false;
    // }
    return true;
}

// check mem
bool isa_difftest_checkmem(uint8_t *ref_m, vaddr_t pc)
{
    for (int i = 0; i < CONFIG_MSIZE; i++)
    {
        if (ref_m[i] != pmem[i])
        {
            printf(ANSI_BG_RED "memory of NPC is different before executing instruction at pc = " FMT_WORD
                               ", mem[%x] right = " FMT_WORD ", wrong = " FMT_WORD ", diff = " FMT_WORD ANSI_NONE "\n",
                   sim_cpu.pc, i, ref_m[i], pmem[i], ref_m[i] ^ pmem[i]);
            return false;
        }
    }
    return true;
}

static void checkregs(CPU_state *ref, vaddr_t pc)
{
    if (!isa_difftest_checkregs(ref, pc))
    {
        sim_state.state = SIM_ABORT;
        sim_state.halt_pc = pc;
        isa_reg_display();
    }
}

static void checkmem(uint8_t *ref_m, vaddr_t pc)
{
    if (!isa_difftest_checkmem(ref_m, pc))
    {
        
        sim_state.state = SIM_ABORT;
        sim_state.halt_pc = pc;
    }
}

void difftest_step()
{
    CPU_state ref_r;
    difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
    // printf("difftest_regcpy\n");
    // difftest_memcpy(CONFIG_MBASE, ref_pmem, CONFIG_MSIZE, DIFFTEST_TO_DUT);
    checkregs(&ref_r, sim_cpu.pc);
    // printf("checkregs\n");
    difftest_exec(1);
    // printf("difftest_exec\n");
    // checkmem(ref_pmem, sim_cpu.pc);
}