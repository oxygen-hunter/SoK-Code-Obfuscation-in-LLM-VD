#include "lzw.h"

#define MAX_CODES (1 << LZW_CODE_MAX)

typedef struct
{
        guint8 index;
        guint16 extends;
} LZWCode;

struct _LZWDecoder
{
        GObject parent_instance;
        int min_code_size;
        int code_size;
        int clear_code;
        int eoi_code;
        LZWCode code_table[MAX_CODES];
        int code_table_size;
        int code;
        int code_bits;
        int last_code;
};

G_DEFINE_TYPE (LZWDecoder, lzw_decoder, G_TYPE_OBJECT)

enum Instructions {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
};

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int instructions[1024];
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm) {
    while (1) {
        switch (vm->instructions[vm->pc++]) {
            case PUSH:
                vm_push(vm, vm->instructions[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = vm->instructions[vm->pc];
                break;
            case JZ: {
                int addr = vm->instructions[vm->pc++];
                if (vm_pop(vm) == 0) vm->pc = addr;
                break;
            }
            case LOAD: {
                int reg = vm->instructions[vm->pc++];
                vm_push(vm, reg);
                break;
            }
            case STORE: {
                int reg = vm->instructions[vm->pc++];
                reg = vm_pop(vm);
                break;
            }
            case HALT:
                return;
        }
    }
}

void add_code_vm(VM *vm, LZWDecoder *self) {
    vm_init(vm);
    vm->instructions[0] = PUSH;
    vm->instructions[1] = self->code;
    vm->instructions[2] = LOAD;
    vm->instructions[3] = self->code;
    vm->instructions[4] = LOAD;
    vm->instructions[5] = self->eoi_code;
    vm->instructions[6] = JZ;
    vm->instructions[7] = 12;
    vm->instructions[8] = LOAD;
    vm->instructions[9] = self->code_table_size;
    vm->instructions[10] = STORE;
    vm->instructions[11] = self->code_table_size;
    vm->instructions[12] = HALT;
    vm_run(vm);
}

gsize write_indexes_vm(VM *vm, LZWDecoder *self, guint8 *output, gsize output_length) {
    vm_init(vm);
    vm->instructions[0] = PUSH;
    vm->instructions[1] = self->code;
    vm->instructions[2] = LOAD;
    vm->instructions[3] = self->code;
    vm->instructions[4] = LOAD;
    vm->instructions[5] = self->code_table_size;
    vm->instructions[6] = JZ;
    vm->instructions[7] = 14;
    vm->instructions[8] = LOAD;
    vm->instructions[9] = output_length;
    vm->instructions[10] = STORE;
    vm->instructions[11] = self->code;
    vm->instructions[12] = STORE;
    vm->instructions[13] = output_length;
    vm->instructions[14] = HALT;
    vm_run(vm);
    return vm_pop(vm);
}

void lzw_decoder_class_init (LZWDecoderClass *klass) {}

void lzw_decoder_init (LZWDecoder *self) {}

LZWDecoder *lzw_decoder_new (guint8 code_size) {
    LZWDecoder *self;
    int i;
    self = g_object_new (lzw_decoder_get_type (), NULL);
    self->min_code_size = code_size;
    self->code_size = code_size;
    self->clear_code = 1 << (code_size - 1);
    self->eoi_code = self->clear_code + 1;
    for (i = 0; i <= self->eoi_code; i++) {
        self->code_table[i].index = i;
        self->code_table[i].extends = self->eoi_code;
        self->code_table_size++;
    }
    self->code = 0;
    self->last_code = self->clear_code;
    return self;
}

gsize lzw_decoder_feed (LZWDecoder *self, guint8 *input, gsize input_length, guint8 *output, gsize output_length) {
    VM vm;
    gsize n_written = 0;
    g_return_val_if_fail (LZW_IS_DECODER (self), 0);
    if (self->last_code == self->eoi_code)
        return 0;
    for (gsize i = 0; i < input_length; i++) {
        guint8 d = input[i];
        int n_available;
        for (n_available = 8; n_available > 0; ) {
            int n_bits, new_bits;
            n_bits = MIN (self->code_size - self->code_bits, n_available);
            new_bits = d & ((1 << n_bits) - 1);
            d = d >> n_bits;
            n_available -= n_bits;
            self->code = new_bits << self->code_bits | self->code;
            self->code_bits += n_bits;
            if (self->code_bits < self->code_size)
                continue;
            if (self->code == self->eoi_code) {
                self->last_code = self->code;
                return n_written;
            }
            if (self->code == self->clear_code) {
                self->code_table_size = self->eoi_code + 1;
                self->code_size = self->min_code_size;
            } else {
                if (self->last_code != self->clear_code && self->code_table_size < MAX_CODES) {
                    if (self->code < self->code_table_size)
                        add_code_vm(&vm, self);
                    else if (self->code == self->code_table_size)
                        add_code_vm(&vm, self);
                    else {
                        self->last_code = self->eoi_code;
                        return output_length;
                    }
                    if (self->code_table_size == (1 << self->code_size) && self->code_size < LZW_CODE_MAX)
                        self->code_size++;
                }
                n_written += write_indexes_vm(&vm, self, output + n_written, output_length - n_written);
            }
            self->last_code = self->code;
            self->code = 0;
            self->code_bits = 0;
            if (n_written >= output_length)
                return output_length;
        }
    }
    return n_written;
}