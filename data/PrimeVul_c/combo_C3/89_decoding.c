#include "gif.h"

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

typedef struct {
    Instruction instr;
    int operand;
} ByteCode;

typedef struct {
    int stack[256];
    int sp; // stack pointer
    int pc; // program counter
    ByteCode *program;
    GifInfo *info;
    bool exitAfterFrame;
} VM;

void initVM(VM *vm, ByteCode *program, GifInfo *info, bool exitAfterFrame) {
    vm->sp = -1;
    vm->pc = 0;
    vm->program = program;
    vm->info = info;
    vm->exitAfterFrame = exitAfterFrame;
}

void push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void execute(VM *vm) {
    for (;;) {
        ByteCode *instr = &vm->program[vm->pc++];
        switch (instr->instr) {
            case PUSH:
                push(vm, instr->operand);
                break;
            case POP:
                pop(vm);
                break;
            case ADD: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a + b);
                break;
            }
            case SUB: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = instr->operand;
                break;
            case JZ: {
                int condition = pop(vm);
                if (condition == 0) {
                    vm->pc = instr->operand;
                }
                break;
            }
            case LOAD: {
                int index = instr->operand;
                push(vm, vm->info->gifFilePtr->ImageCount);
                break;
            }
            case STORE: {
                int index = instr->operand;
                vm->info->gifFilePtr->ImageCount = pop(vm);
                break;
            }
            case CALL:
                push(vm, vm->pc);
                vm->pc = instr->operand;
                break;
            case RET:
                vm->pc = pop(vm);
                break;
            case HALT:
                return;
        }
    }
}

void DDGifSlurp(GifInfo *info, bool decode, bool exitAfterFrame) {
    ByteCode program[] = {
        {PUSH, 0}, 
        {LOAD, 0}, 
        {PUSH, 1}, 
        {SUB, 0}, 
        {JZ, 10}, 
        {PUSH, 0}, 
        {STORE, 0}, 
        {LOAD, 0}, 
        {PUSH, 2}, 
        {ADD, 0}, 
        {STORE, 0}, 
        {HALT, 0}
    };
    
    VM vm;
    initVM(&vm, program, info, exitAfterFrame);
    execute(&vm);
}

static int readExtensions(int ExtFunction, GifByteType *ExtData, GifInfo *info) {
    if (ExtData == NULL) {
        return GIF_OK;
    }
    if (ExtFunction == GRAPHICS_EXT_FUNC_CODE) {
        GraphicsControlBlock *GCB = &info->controlBlock[info->gifFilePtr->ImageCount];
        if (DGifExtensionToGCB(ExtData[0], ExtData + 1, GCB) == GIF_ERROR) {
            return GIF_ERROR;
        }
        GCB->DelayTime = GCB->DelayTime > 1 ? GCB->DelayTime * 10 : DEFAULT_FRAME_DURATION_MS;
    }
    else if (ExtFunction == COMMENT_EXT_FUNC_CODE) {
        if (getComment(ExtData, info) == GIF_ERROR) {
            info->gifFilePtr->Error = D_GIF_ERR_NOT_ENOUGH_MEM;
            return GIF_ERROR;
        }
    }
    else if (ExtFunction == APPLICATION_EXT_FUNC_CODE) {
        char const *string = (char const *) (ExtData + 1);
        if (strncmp("NETSCAPE2.0", string, ExtData[0]) == 0 || strncmp("ANIMEXTS1.0", string, ExtData[0]) == 0) {
            if (DGifGetExtensionNext(info->gifFilePtr, &ExtData) == GIF_ERROR) {
                return GIF_ERROR;
            }
            if (ExtData && ExtData[0] == 3 && ExtData[1] == 1) {
                uint_fast16_t loopCount = (uint_fast16_t) (ExtData[2] + (ExtData[3] << 8));
                if (loopCount) {
                    loopCount++;
                }
                info->loopCount = loopCount;
            }
        }
    }
    return GIF_OK;
}

static int getComment(GifByteType *Bytes, GifInfo *info) {
    unsigned int len = (unsigned int) Bytes[0];
    size_t offset = info->comment != NULL ? strlen(info->comment) : 0;
    char *ret = reallocarray(info->comment, len + offset + 1, sizeof(char));
    if (ret != NULL) {
        memcpy(ret + offset, &Bytes[1], len);
        ret[len + offset] = 0;
        info->comment = ret;
        return GIF_OK;
    }
    info->gifFilePtr->Error = D_GIF_ERR_NOT_ENOUGH_MEM;
    return GIF_ERROR;
}