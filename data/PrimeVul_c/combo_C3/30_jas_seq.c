#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int *data_;
    int *rows_;
    int numrows_;
    int numcols_;
    int xstart_;
    int ystart_;
    int xend_;
    int yend_;
    int maxrows_;
    int datasize_;
    int flags_;
} jas_matrix_t;

#define JAS_MATRIX_REF 1
#define JAS_CAST(type, val) ((type)(val))

typedef int jas_seqent_t;
typedef jas_matrix_t jas_seq2d_t;

enum {
    OP_PUSH, OP_POP, OP_ADD, OP_SUB, OP_JMP, OP_JZ, OP_LOAD, OP_STORE, OP_HALT
};

typedef struct {
    int *stack;
    int sp;
    int pc;
    int *program;
} VM;

VM *vm_create(int *program) {
    VM *vm = malloc(sizeof(VM));
    vm->stack = malloc(1024 * sizeof(int));
    vm->sp = -1;
    vm->pc = 0;
    vm->program = program;
    return vm;
}

void vm_destroy(VM *vm) {
    free(vm->stack);
    free(vm);
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm) {
    int running = 1;
    while (running) {
        int instr = vm->program[vm->pc++];
        switch (instr) {
            case OP_PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case OP_POP:
                vm_pop(vm);
                break;
            case OP_ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case OP_SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case OP_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case OP_JZ: {
                int addr = vm->program[vm->pc++];
                if (vm_pop(vm) == 0)
                    vm->pc = addr;
                break;
            }
            case OP_LOAD: {
                int addr = vm->program[vm->pc++];
                vm_push(vm, vm->stack[addr]);
                break;
            }
            case OP_STORE: {
                int addr = vm->program[vm->pc++];
                vm->stack[addr] = vm_pop(vm);
                break;
            }
            case OP_HALT:
                running = 0;
                break;
            default:
                break;
        }
    }
}

jas_matrix_t *jas_matrix_create(int numrows, int numcols) {
    jas_matrix_t *matrix = malloc(sizeof(jas_matrix_t));
    int program[] = {
        OP_PUSH, numrows,
        OP_PUSH, numcols,
        OP_MUL,
        OP_STORE, 0,
        OP_PUSH, sizeof(jas_matrix_t),
        OP_MUL,
        OP_PUSH, 1,
        OP_MUL,
        OP_ADD,
        OP_STORE, 1,
        OP_LOAD, 0,
        OP_LOAD, 1,
        OP_PUSH, 0,
        OP_STORE, 2,
        OP_HALT
    };
    VM *vm = vm_create(program);
    vm_run(vm);
    free(vm);
    return matrix;
}

void jas_matrix_destroy(jas_matrix_t *matrix) {
    if (matrix->data_) {
        assert(!(matrix->flags_ & JAS_MATRIX_REF));
        free(matrix->data_);
        matrix->data_ = 0;
    }
    if (matrix->rows_) {
        free(matrix->rows_);
        matrix->rows_ = 0;
    }
    free(matrix);
}

int main() {
    jas_matrix_t *matrix = jas_matrix_create(5, 5);
    jas_matrix_destroy(matrix);
    return 0;
}