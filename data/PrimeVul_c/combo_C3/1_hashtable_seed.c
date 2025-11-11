#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define PUSH 0
#define POP 1
#define ADD 2
#define SUB 3
#define LOAD 4
#define STORE 5
#define JMP 6
#define JZ 7
#define CALL 8
#define RET 9
#define HALT 10

#define STACK_SIZE 256
#define MEMORY_SIZE 256
#define PROGRAM_SIZE 1024

typedef struct {
    int pc;
    int sp;
    int stack[STACK_SIZE];
    int memory[MEMORY_SIZE];
    int program[PROGRAM_SIZE];
} VM;

void init_vm(VM* vm) {
    vm->pc = 0;
    vm->sp = -1;
    for (int i = 0; i < STACK_SIZE; i++) vm->stack[i] = 0;
    for (int i = 0; i < MEMORY_SIZE; i++) vm->memory[i] = 0;
}

void push(VM* vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM* vm) {
    return vm->stack[vm->sp--];
}

void execute(VM* vm) {
    int running = 1;
    while (running) {
        int instruction = vm->program[vm->pc++];
        switch (instruction) {
        case PUSH:
            push(vm, vm->program[vm->pc++]);
            break;
        case POP:
            pop(vm);
            break;
        case ADD:
            push(vm, pop(vm) + pop(vm));
            break;
        case SUB:
            push(vm, pop(vm) - pop(vm));
            break;
        case LOAD:
            push(vm, vm->memory[vm->program[vm->pc++]]);
            break;
        case STORE:
            vm->memory[vm->program[vm->pc++]] = pop(vm);
            break;
        case JMP:
            vm->pc = vm->program[vm->pc];
            break;
        case JZ:
            if (pop(vm) == 0) vm->pc = vm->program[vm->pc];
            else vm->pc++;
            break;
        case CALL:
            push(vm, vm->pc + 1);
            vm->pc = vm->program[vm->pc];
            break;
        case RET:
            vm->pc = pop(vm);
            break;
        case HALT:
            running = 0;
            break;
        }
    }
}

void generate_seed_vm(VM* vm) {
    vm->program[0] = CALL; vm->program[1] = 10;
    vm->program[2] = STORE; vm->program[3] = 0;
    vm->program[4] = LOAD; vm->program[5] = 0;
    vm->program[6] = JZ; vm->program[7] = 9;
    vm->program[8] = HALT;
    vm->program[9] = PUSH; vm->program[10] = 1;
    vm->program[11] = RET;
    vm->program[12] = PUSH; vm->program[13] = 0;
    vm->program[14] = RET;

    execute(vm);
}

uint32_t buf_to_uint32(char* data) {
    VM vm;
    init_vm(&vm);
    
    for (size_t i = 0; i < sizeof(uint32_t); i++) {
        vm.program[i] = PUSH;
        vm.program[i + 1] = (unsigned char)data[i];
    }
    
    vm.program[sizeof(uint32_t) * 2] = HALT;
    execute(&vm);

    uint32_t result = 0;
    for (size_t i = 0; i < sizeof(uint32_t); i++) {
        result = (result << 8) | (unsigned char)pop(&vm);
    }

    return result;
}

int seed_from_urandom(uint32_t* seed) {
    char data[sizeof(uint32_t)];
    int urandom = open("/dev/urandom", O_RDONLY);
    if (urandom == -1) return 1;

    int ok = read(urandom, data, sizeof(uint32_t)) == sizeof(uint32_t);
    close(urandom);

    if (!ok) return 1;

    *seed = buf_to_uint32(data);
    return 0;
}

int seed_from_timestamp_and_pid(uint32_t* seed) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *seed = (uint32_t)tv.tv_sec ^ (uint32_t)tv.tv_usec;
    *seed ^= (uint32_t)getpid();
    return 0;
}

uint32_t generate_seed() {
    uint32_t seed;
    int done = 0;

    if (!done && seed_from_urandom(&seed) == 0) done = 1;
    if (!done) seed_from_timestamp_and_pid(&seed);

    if (seed == 0) seed = 1;

    return seed;
}

volatile uint32_t hashtable_seed = 0;

void json_object_seed(size_t seed) {
    uint32_t new_seed = (uint32_t)seed;

    if (hashtable_seed == 0) {
        if (new_seed == 0) new_seed = generate_seed();
        hashtable_seed = new_seed;
    }
}