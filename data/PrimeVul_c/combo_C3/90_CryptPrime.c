#include "Tpm.h"
#include "CryptPrime_fp.h"
#include "CryptPrimeSieve_fp.h"

extern const uint32_t s_LastPrimeInTable;
extern const uint32_t s_PrimeTableSize;
extern const uint32_t s_PrimesInTable;
extern const unsigned char s_PrimeTable[];
extern bigConst s_CompositeOfSmallPrimes;

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int code[1024];
} VM;

void vm_init(VM* vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_push(VM* vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM* vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VM* vm) {
    int running = 1;
    while(running) {
        switch(vm->code[vm->pc++]) {
            case PUSH: vm_push(vm, vm->code[vm->pc++]); break;
            case POP: vm_pop(vm); break;
            case ADD: vm_push(vm, vm_pop(vm) + vm_pop(vm)); break;
            case SUB: { int b = vm_pop(vm); int a = vm_pop(vm); vm_push(vm, a - b); } break;
            case JMP: vm->pc = vm->code[vm->pc]; break;
            case JZ: if(vm_pop(vm) == 0) vm->pc = vm->code[vm->pc]; else vm->pc++; break;
            case LOAD: vm_push(vm, vm->stack[vm->code[vm->pc++]]); break;
            case STORE: vm->stack[vm->code[vm->pc++]] = vm_pop(vm); break;
            case CALL: { int addr = vm->code[vm->pc++]; vm_push(vm, vm->pc); vm->pc = addr; } break;
            case RET: vm->pc = vm_pop(vm); break;
            case HALT: running = 0; break;
        }
    }
}

void prime_validator(VM* vm, int n) {
    vm->code[0] = PUSH; vm->code[1] = n;
    vm->code[2] = PUSH; vm->code[3] = 2;
    vm->code[4] = SUB;
    vm->code[5] = JZ; vm->code[6] = 14; 
    vm->code[7] = PUSH; vm->code[8] = n;
    vm->code[9] = PUSH; vm->code[10] = 1;
    vm->code[11] = ADD;
    vm->code[12] = HALT;
    vm->code[13] = HALT;
    vm->code[14] = HALT;

    vm_execute(vm);
}

int main() {
    VM vm;
    vm_init(&vm);
    prime_validator(&vm, 29);
    return 0;
}