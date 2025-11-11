import argparse
import ctypes
from ctypes import c_char_p, c_void_p, CDLL

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=True)
    parser.add_argument("--input-path", required=False)
    parser.add_argument("--output-path", required=False)
    parser.add_argument("--content-type", required=True)
    return parser.parse_args()

def load_c_library():
    # Assuming the C shared library is named 'mlflow_predictor.so'
    return CDLL('./mlflow_predictor.so')

def c_predict(model_uri, input_path, output_path, content_type):
    lib = load_c_library()
    lib.predict.argtypes = [c_char_p, c_char_p, c_char_p, c_char_p]
    lib.predict.restype = c_void_p
    lib.predict(
        model_uri.encode('utf-8'),
        input_path.encode('utf-8') if input_path else None,
        output_path.encode('utf-8') if output_path else None,
        content_type.encode('utf-8')
    )

def main():
    args = parse_args()
    c_predict(
        model_uri=args.model_uri,
        input_path=args.input_path if args.input_path else None,
        output_path=args.output_path if args.output_path else None,
        content_type=args.content_type,
    )

if __name__ == "__main__":
    main()
```

```c
// This is the C code that should be compiled into 'mlflow_predictor.so'
#include <stdio.h>

void predict(const char* model_uri, const char* input_path, const char* output_path, const char* content_type) {
    // Placeholder for the actual prediction logic
    printf("Model URI: %s\n", model_uri);
    if (input_path) {
        printf("Input Path: %s\n", input_path);
    }
    if (output_path) {
        printf("Output Path: %s\n", output_path);
    }
    printf("Content Type: %s\n", content_type);
}