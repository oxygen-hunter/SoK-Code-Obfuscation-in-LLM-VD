#include "iwl-trans.h"
#include "iwl-fh.h"
#include "iwl-context-info-gen3.h"
#include "internal.h"
#include "iwl-prph.h"

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int running;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_execute(VM *vm, int *program) {
    while (vm->running) {
        switch (program[vm->pc++]) {
            case PUSH: vm->stack[++vm->sp] = program[vm->pc++]; break;
            case POP: vm->sp--; break;
            case ADD: vm->stack[vm->sp-1] += vm->stack[vm->sp]; vm->sp--; break;
            case SUB: vm->stack[vm->sp-1] -= vm->stack[vm->sp]; vm->sp--; break;
            case JMP: vm->pc = program[vm->pc]; break;
            case JZ: if (vm->stack[vm->sp--] == 0) vm->pc = program[vm->pc]; else vm->pc++; break;
            case LOAD: vm->stack[++vm->sp] = program[program[vm->pc++]]; break;
            case STORE: program[program[vm->pc++]] = vm->stack[vm->sp--]; break;
            case CALL: vm->stack[++vm->sp] = vm->pc + 1; vm->pc = program[vm->pc]; break;
            case RET: vm->pc = vm->stack[vm->sp--]; break;
            case HALT: vm->running = 0; break;
        }
    }
}

int iwl_pcie_ctxt_info_gen3_init(struct iwl_trans *trans, const struct fw_img *fw) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        PUSH, IWL_CMD_QUEUE_SIZE, PUSH, trans->cfg->min_txq_size, CALL, 100,
        LOAD, 200, JZ, 300, HALT,
        // Max function
        100, LOAD, 0, LOAD, 1, SUB, JZ, 110, LOAD, 0, RET,
        110, LOAD, 1, RET,
        // DMA allocation and control flags setup
        200, PUSH, (int)trans->dev, PUSH, sizeof(struct iwl_prph_scratch), CALL, 400,
        STORE, 0, JZ, 300, PUSH, IWL_PRPH_SCRATCH_RB_SIZE_4K, PUSH, IWL_PRPH_SCRATCH_MTR_MODE, ADD,
        PUSH, IWL_PRPH_MTR_FORMAT_256B, PUSH, IWL_PRPH_SCRATCH_MTR_FORMAT, AND, ADD,
        PUSH, IWL_PRPH_SCRATCH_EARLY_DEBUG_EN, ADD, PUSH, IWL_PRPH_SCRATCH_EDBG_DEST_DRAM, ADD,
        STORE, 1, HALT,
        // DMA allocation
        400, PUSH, (int)dma_alloc_coherent, CALL, 500, RET,
        // DMA allocation internal
        500, HALT
    };
    vm_execute(&vm, program);
    return program[0] == 0 ? -ENOMEM : 0;
}

void iwl_pcie_ctxt_info_gen3_free(struct iwl_trans *trans) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        PUSH, (int)trans, CALL, 200, HALT,
        // Free function
        200, PUSH, (int)dma_free_coherent, CALL, 300, RET,
        // DMA free internal
        300, HALT
    };
    vm_execute(&vm, program);
}