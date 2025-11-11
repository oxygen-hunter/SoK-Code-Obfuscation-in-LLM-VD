# -*- coding: utf-8 -*-

import random
import string

import js2py

js2py.disable_pyimport()

def random_string(length):
    seq = string.ascii_letters + string.digits + string.punctuation
    return "".join(random.choice(seq) for _ in range(length))

def is_plural(value):
    try:
        n = abs(float(value))
        return n == 0 or n > 1
    except ValueError:
        return value.endswith("s")

def eval_js(script, es6=False):
    return (js2py.eval_js6 if es6 else js2py.eval_js)(script)

def accumulate(iterable, to_map=None):
    if to_map is None:
        to_map = {}
    for key, value in iterable:
        to_map.setdefault(value, []).append(key)
    return to_map

def reversemap(obj):
    return obj.__class__(reversed(item) for item in obj.items())

```

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 1024

char *random_string(int length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+";
    char *randomString = NULL;
    if (length) {
        randomString = malloc(length + 1);
        if (randomString) {
            for (int n = 0; n < length; n++) {
                int key = rand() % (int)(sizeof(charset) - 1);
                randomString[n] = charset[key];
            }
            randomString[length] = '\0';
        }
    }
    return randomString;
}

int is_plural(const char *value) {
    double n;
    if (sscanf(value, "%lf", &n) == 1) {
        return n == 0 || n > 1;
    } else {
        return value[strlen(value) - 1] == 's';
    }
}

int main() {
    char *randStr = random_string(10);
    printf("Random String: %s\n", randStr);
    printf("Is plural: %d\n", is_plural("cats"));
    free(randStr);
    return 0;
}
```

```solidity
pragma solidity ^0.8.0;

contract JSExecutor {
    function eval_js(string memory script, bool es6) public pure returns (string memory) {
        assembly {
            let result := mload(0x40) // Get the free memory pointer
            mstore(result, script) // Store the script at the result location
            mstore(0x40, add(result, 0x20)) // Update the free memory pointer

            switch es6
            case true {
                // Inline assembly for ES6 evaluation
            }
            default {
                // Inline assembly for ES5 evaluation
            }

            return(result, 0x20) // Return the result
        }
    }
}