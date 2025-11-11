#include <gnutls_int.h>
#include <gnutls_mpi.h>
#include <gnutls_ecc.h>
#include <algorithms.h>
#include <gnutls_errors.h>

typedef enum { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT } OpCode;
typedef struct { OpCode op; int arg; } Instruction;
typedef struct { int stack[256]; int sp; int pc; int halted; } VM;

void execute(VM* vm, Instruction* program) {
    while (!vm->halted) {
        Instruction instr = program[vm->pc++];
        switch (instr.op) {
            case PUSH: vm->stack[vm->sp++] = instr.arg; break;
            case POP: vm->sp--; break;
            case ADD: vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1]; vm->sp--; break;
            case SUB: vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1]; vm->sp--; break;
            case JMP: vm->pc = instr.arg; break;
            case JZ: if (vm->stack[--vm->sp] == 0) vm->pc = instr.arg; break;
            case LOAD: vm->stack[vm->sp++] = vm->stack[instr.arg]; break;
            case STORE: vm->stack[instr.arg] = vm->stack[--vm->sp]; break;
            case HALT: vm->halted = 1; break;
        }
    }
}

int _gnutls_ecc_ansi_x963_export(gnutls_ecc_curve_t curve, bigint_t x, bigint_t y, gnutls_datum_t* out) {
    int numlen = gnutls_ecc_curve_get_size(curve);
    if (numlen == 0) return gnutls_assert_val(GNUTLS_E_INVALID_REQUEST);
    out->size = 1 + 2 * numlen;
    out->data = gnutls_malloc(out->size);
    if (out->data == NULL) return gnutls_assert_val(GNUTLS_E_MEMORY_ERROR);
    
    Instruction program[] = {
        { PUSH, 0x04 },
        { STORE, 0 },
        { PUSH, (_gnutls_mpi_get_nbits(x) + 7) / 8 },
        { PUSH, out->size - (1 + (numlen - ((_gnutls_mpi_get_nbits(x) + 7) / 8))) },
        { CALL, 1 },
        { JZ, 20 },
        { PUSH, (_gnutls_mpi_get_nbits(y) + 7) / 8 },
        { PUSH, out->size - (1 + (numlen + numlen - ((_gnutls_mpi_get_nbits(y) + 7) / 8))) },
        { CALL, 1 },
        { JZ, 20 },
        { PUSH, 0 },
        { JMP, 21 },
        { HALT, 0 }
    };

    VM vm = { .sp = 0, .pc = 0, .halted = 0 };
    execute(&vm, program);
    return vm.stack[vm.sp - 1];
}

int _gnutls_ecc_ansi_x963_import(const uint8_t* in, unsigned long inlen, bigint_t* x, bigint_t* y) {
    if ((inlen & 1) == 0) return GNUTLS_E_INVALID_REQUEST;
    if (in[0] != 4) return gnutls_assert_val(GNUTLS_E_PARSING_ERROR);

    Instruction program[] = {
        { PUSH, inlen },
        { PUSH, 1 },
        { SUB, 0 },
        { PUSH, 2 },
        { DIV, 0 },
        { CALL, 2 },
        { JZ, 10 },
        { PUSH, in + 1 + ((inlen - 1) >> 1) },
        { CALL, 2 },
        { JZ, 10 },
        { PUSH, 0 },
        { JMP, 11 },
        { HALT, 0 }
    };

    VM vm = { .sp = 0, .pc = 0, .halted = 0 };
    execute(&vm, program);
    return vm.stack[vm.sp - 1];
}

int _gnutls_ecc_curve_fill_params(gnutls_ecc_curve_t curve, gnutls_pk_params_st* params) {
    const gnutls_ecc_curve_entry_st* st = _gnutls_ecc_curve_get_params(curve);
    if (st == NULL) return gnutls_assert_val(GNUTLS_E_ECC_UNSUPPORTED_CURVE);
    uint8_t val[MAX_ECC_CURVE_SIZE];
    size_t val_size = sizeof(val);
    int ret;

    Instruction program[] = {
        { PUSH, st->prime },
        { PUSH, strlen(st->prime) },
        { PUSH, val },
        { PUSH, &val_size },
        { CALL, 3 },
        { JZ, 40 },
        { PUSH, params->params + ECC_PRIME },
        { PUSH, val },
        { PUSH, val_size },
        { CALL, 4 },
        { JZ, 40 },
        { INC, params->params_nr },
        { HALT, 0 }
    };

    VM vm = { .sp = 0, .pc = 0, .halted = 0 };
    execute(&vm, program);
    return vm.stack[vm.sp - 1];

cleanup:
    gnutls_pk_params_release(params);
    return ret;
}