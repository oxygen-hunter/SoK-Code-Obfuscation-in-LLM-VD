import argparse
import ctypes
import os

def py_parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=True)
    parser.add_argument("--input-path", required=False)
    parser.add_argument("--output-path", required=False)
    parser.add_argument("--content-type", required=True)
    return parser.parse_args()

def c_predict(model_uri, input_path, output_path, content_type):
    lib = ctypes.CDLL(os.path.join(os.path.dirname(__file__), "libpredict.so"))
    c_model_uri = ctypes.c_char_p(model_uri.encode('utf-8'))
    c_input_path = ctypes.c_char_p(input_path.encode('utf-8')) if input_path else ctypes.c_char_p(None)
    c_output_path = ctypes.c_char_p(output_path.encode('utf-8')) if output_path else ctypes.c_char_p(None)
    c_content_type = ctypes.c_char_p(content_type.encode('utf-8'))
    lib.predict(c_model_uri, c_input_path, c_output_path, c_content_type)

def main():
    args = py_parse_args()
    c_predict(
        model_uri=args.model_uri,
        input_path=args.input_path if args.input_path else "",
        output_path=args.output_path if args.output_path else "",
        content_type=args.content_type,
    )

if __name__ == "__main__":
    main()
```

```c
// libpredict.c
#include <stdio.h>

void predict(const char* model_uri, const char* input_path, const char* output_path, const char* content_type) {
    // Simulate prediction logic
    printf("Model URI: %s\n", model_uri);
    printf("Input Path: %s\n", input_path);
    printf("Output Path: %s\n", output_path);
    printf("Content Type: %s\n", content_type);
}
```

```bash
# Compile the C code
gcc -shared -o libpredict.so -fPIC libpredict.c