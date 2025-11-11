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

G_DEFINE_TYPE(LZWDecoder, lzw_decoder, G_TYPE_OBJECT)

static void
add_code(LZWDecoder *self, int code)
{
        int c = code;
        while (self->code_table[c].extends != self->eoi_code)
                c = self->code_table[c].extends;

        self->code_table[self->code_table_size].index = self->code_table[c].index;
        self->code_table[self->code_table_size].extends = self->last_code;
        self->code_table_size++;
}

static gsize
write_indexes_rec(LZWDecoder *self, guint8 *output, gsize output_length, int c, gsize offset)
{
        if (offset >= output_length)
                return offset + 1;

        output[offset] = self->code_table[c].index;

        if (self->code_table[c].extends == self->eoi_code)
                return offset + 1;

        return write_indexes_rec(self, output, output_length, self->code_table[c].extends, offset - 1);
}

static gsize
write_indexes(LZWDecoder *self, guint8 *output, gsize output_length)
{
        int c;
        gsize index_count = 1;

        if (self->code >= self->code_table_size)
                return 0;

        c = self->code;
        while (self->code_table[c].extends != self->eoi_code) {
                c = self->code_table[c].extends;
                index_count++;
        }

        return write_indexes_rec(self, output, output_length, self->code, index_count - 1);
}

void
lzw_decoder_class_init(LZWDecoderClass *klass)
{
}

void
lzw_decoder_init(LZWDecoder *self)
{
}

LZWDecoder *
lzw_decoder_new(guint8 code_size)
{
        LZWDecoder *self;
        int i;

        self = g_object_new(lzw_decoder_get_type(), NULL);

        self->min_code_size = code_size;
        self->code_size = code_size;

        self->clear_code = 1 << (code_size - 1);
        self->eoi_code = self->clear_code + 1;

        i = 0;
        while (i <= self->eoi_code) {
                self->code_table[i].index = i;
                self->code_table[i].extends = self->eoi_code;
                self->code_table_size++;
                i++;
        }

        self->code = 0;
        self->last_code = self->clear_code;

        return self;
}

static gsize
process_bits(LZWDecoder *self, guint8 d, int n_available, guint8 *output, gsize output_length, gsize *n_written)
{
        if (n_available <= 0)
                return *n_written;

        int n_bits, new_bits;

        n_bits = MIN(self->code_size - self->code_bits, n_available);
        new_bits = d & ((1 << n_bits) - 1);
        d = d >> n_bits;
        n_available -= n_bits;

        self->code = new_bits << self->code_bits | self->code;
        self->code_bits += n_bits;
        if (self->code_bits < self->code_size)
                return process_bits(self, d, n_available, output, output_length, n_written);

        if (self->code == self->eoi_code) {
                self->last_code = self->code;
                return *n_written;
        }

        if (self->code == self->clear_code) {
                self->code_table_size = self->eoi_code + 1;
                self->code_size = self->min_code_size;
        } else {
                if (self->last_code != self->clear_code && self->code_table_size < MAX_CODES) {
                        if (self->code < self->code_table_size)
                                add_code(self, self->code);
                        else if (self->code == self->code_table_size)
                                add_code(self, self->last_code);
                        else {
                                self->last_code = self->eoi_code;
                                return output_length;
                        }

                        if (self->code_table_size == (1 << self->code_size) && self->code_size < LZW_CODE_MAX)
                                self->code_size++;
                }

                *n_written += write_indexes(self, output + *n_written, output_length - *n_written);
        }

        self->last_code = self->code;
        self->code = 0;
        self->code_bits = 0;

        if (*n_written >= output_length)
                return output_length;

        return process_bits(self, d, n_available, output, output_length, n_written);
}

static gsize
lzw_decoder_feed_rec(LZWDecoder *self, guint8 *input, gsize input_length, guint8 *output, gsize output_length, gsize i, gsize *n_written)
{
        if (i >= input_length)
                return *n_written;

        guint8 d = input[i];

        *n_written = process_bits(self, d, 8, output, output_length, n_written);

        return lzw_decoder_feed_rec(self, input, input_length, output, output_length, i + 1, n_written);
}

gsize
lzw_decoder_feed(LZWDecoder *self, guint8 *input, gsize input_length, guint8 *output, gsize output_length)
{
        gsize n_written = 0;

        g_return_val_if_fail(LZW_IS_DECODER(self), 0);

        if (self->last_code == self->eoi_code)
                return 0;

        return lzw_decoder_feed_rec(self, input, input_length, output, output_length, 0, &n_written);
}