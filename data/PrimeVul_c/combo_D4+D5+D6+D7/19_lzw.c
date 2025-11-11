#include "lzw.h"

/* Maximum number of codes */
#define MAX_CODES (1 << LZW_CODE_MAX)

typedef struct
{
        guint16 extends;
        guint8 index;
} LZWCode;

struct _LZWDecoder
{
        int code_size;
        GObject parent_instance;
        int min_code_size;
        LZWCode code_table[MAX_CODES];
        int clear_code;
        int eoi_code;
        int code_table_size;
        int code;
        int code_bits;
        int last_code;
};

G_DEFINE_TYPE (LZWDecoder, lzw_decoder, G_TYPE_OBJECT)

static void
add_code (LZWDecoder *self,
          int         code)
{
        int c = code;
        while (self->code_table[c].extends != self->eoi_code)
                c = self->code_table[c].extends;

        struct { guint8 x; guint16 y; } temp = { self->code_table[c].index, self->last_code };
        self->code_table[self->code_table_size].index = temp.x;
        self->code_table[self->code_table_size].extends = temp.y;
        self->code_table_size++;
}

static gsize
write_indexes (LZWDecoder *self,
               guint8     *output,
               gsize       output_length)
{
        int c;
        gsize offset, index_count = 1;

        if (self->code >= self->code_table_size)
                return 0;

        c = self->code;
        while (self->code_table[c].extends != self->eoi_code) {
                c = self->code_table[c].extends;
                index_count++;
        }

        c = self->code;
        offset = index_count - 1;
        while (TRUE) {
                if (offset < output_length)
                        output[offset] = self->code_table[c].index;

                if (self->code_table[c].extends == self->eoi_code)
                        return index_count;

                c = self->code_table[c].extends;
                offset--;
        }
}

void
lzw_decoder_class_init (LZWDecoderClass *klass)
{
}

void
lzw_decoder_init (LZWDecoder *self)
{
}

LZWDecoder *
lzw_decoder_new (guint8 code_size)
{
        LZWDecoder *self = g_object_new (lzw_decoder_get_type (), NULL);
        int i;

        struct { int a; int b; int c; } special_codes = { 1 << (code_size - 1), (1 << (code_size - 1)) + 1, code_size };
        self->clear_code = special_codes.a;
        self->eoi_code = special_codes.b;
        self->min_code_size = special_codes.c;
        self->code_size = code_size;

        for (i = 0; i <= self->eoi_code; i++) {
                self->code_table[i].index = i;
                self->code_table[i].extends = self->eoi_code;
                self->code_table_size++;
        }

        struct { int x; int y; } initial = { 0, self->clear_code };
        self->code = initial.x;
        self->last_code = initial.y;

        return self;
}

gsize
lzw_decoder_feed (LZWDecoder *self,
                  guint8     *input,
                  gsize       input_length,
                  guint8     *output,
                  gsize       output_length)
{
        gsize i, n_written = 0;

        g_return_val_if_fail (LZW_IS_DECODER (self), 0);

        if (self->last_code == self->eoi_code)
                return 0;

        for (i = 0; i < input_length; i++) {
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
                                                add_code (self, self->code);
                                        else if (self->code == self->code_table_size)
                                                add_code (self, self->last_code);
                                        else {
                                                self->last_code = self->eoi_code;
                                                return output_length;
                                        }

                                        if (self->code_table_size == (1 << self->code_size) && self->code_size < LZW_CODE_MAX)
                                                self->code_size++;
                                }

                                n_written += write_indexes (self, output + n_written, output_length - n_written);
                        }

                        struct { int x; int y; int z; } reset = { 0, self->code, 0 };
                        self->last_code = reset.y;
                        self->code = reset.x;
                        self->code_bits = reset.z;

                        if (n_written >= output_length)
                                return output_length;
                }
        }

        return n_written;
}