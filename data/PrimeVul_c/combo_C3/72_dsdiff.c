#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#include "wavpack.h"
#include "utils.h"
#include "md5.h"

#ifdef _WIN32
#define strdup(x) _strdup(x)
#endif

#define WAVPACK_NO_ERROR    0
#define WAVPACK_SOFT_ERROR  1
#define WAVPACK_HARD_ERROR  2

extern int debug_logging_mode;

#pragma pack(push,2)

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
} DFFChunkHeader;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    char formType [4];
} DFFFileHeader;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    uint32_t version;
} DFFVersionChunk;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    uint32_t sampleRate;
} DFFSampleRateChunk;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    uint16_t numChannels;
} DFFChannelsHeader;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    char compressionType [4];
} DFFCompressionHeader;

#pragma pack(pop)

#define DFFChunkHeaderFormat "4D"
#define DFFFileHeaderFormat "4D4"
#define DFFVersionChunkFormat "4DL"
#define DFFSampleRateChunkFormat "4DL"
#define DFFChannelsHeaderFormat "4DS"
#define DFFCompressionHeaderFormat "4D4"

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CMP, HALT
};

typedef struct {
    int pc;
    int stack[256];
    int sp;
    int reg[16];
} VM;

void execute(int *program, VM *vm) {
    while (1) {
        int op = program[vm->pc++];
        switch(op) {
            case PUSH: vm->stack[vm->sp++] = program[vm->pc++]; break;
            case POP: vm->sp--; break;
            case ADD: vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1]; vm->sp--; break;
            case SUB: vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1]; vm->sp--; break;
            case JMP: vm->pc = program[vm->pc]; break;
            case JZ: if (vm->stack[--vm->sp] == 0) vm->pc = program[vm->pc]; else vm->pc++; break;
            case LOAD: vm->stack[vm->sp++] = vm->reg[program[vm->pc++]]; break;
            case STORE: vm->reg[program[vm->pc++]] = vm->stack[--vm->sp]; break;
            case CMP: vm->stack[vm->sp - 2] = (vm->stack[vm->sp - 2] == vm->stack[vm->sp - 1]) ? 1 : 0; vm->sp--; break;
            case HALT: return;
        }
    }
}

int ParseDsdiffHeaderConfig (FILE *infile, char *infilename, char *fourcc, WavpackContext *wpc, WavpackConfig *config) {
    int program[] = {
        PUSH, 0, PUSH, 0, PUSH, 0, PUSH, 0, PUSH, 0, PUSH, 0, PUSH, 0,
        LOAD, 0, PUSH, sizeof(DFFFileHeader) - 4, CMP, JZ, 40,
        PUSH, (int)"%s is not a valid .DFF file!", PUSH, (int)infilename, ADD, PUSH, WAVPACK_SOFT_ERROR, HALT,
        PUSH, 1, JZ, 60,
        CMP, JZ, 80,
        PUSH, (int)"%s", PUSH, (int)WavpackGetErrorMessage(wpc), ADD, PUSH, WAVPACK_SOFT_ERROR, HALT,
        PUSH, 0, JZ, 100,
        PUSH, WAVPACK_NO_ERROR, HALT
    };

    VM vm = {0};
    execute(program, &vm);
    return vm.reg[0];
}

int WriteDsdiffHeader (FILE *outfile, WavpackContext *wpc, int64_t total_samples, int qmode) {
    int program[] = {
        PUSH, 0, PUSH, 0, PUSH, 0, PUSH, 0, PUSH, 0, PUSH, 0, PUSH, 0,
        LOAD, 0, PUSH, 0, CMP, JZ, 40,
        PUSH, (int)"can't allocate memory!", PUSH, 0, ADD, HALT,
        PUSH, 0, JZ, 60,
        PUSH, 0, HALT
    };

    VM vm = {0};
    execute(program, &vm);
    return vm.reg[0];
}