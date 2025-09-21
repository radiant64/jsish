# JSISH - JSON Serialization in Single Header

A C89-compliant single header library for both decoding and encoding JSON data,
without making any heap allocations for either operation. It's been written for
minimalism of both sources and binary size rather than for speed of processing,
though it should perform OK in that regard.

It does not implement any hash tables or binary trees for key-value lookups, so
lookups of specific JSON paths will exhibit linear time complexity. If you need
fast random lookups, it is recommended that you iterate over the decoded JSON
data and store it in some better suited data structure.

## Decoder usage

Usage example (modern C):

```c
#define JSISH_MAIN
#include "jsish.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    // JSON document:
    // {
    //     "key_a": "value a",
    //     "key_b": [1, 2, false]
    // }
    const char* example_json_text
        = "{\"key_a\": \"value a\", \"key_b\": [1, 2, false]}";
    
    // The decoder uses in-situ pointers to string values to avoid allocating
    // space for copies, so the string it parses must be mutable so it can
    // insert terminating zeroes.
    char* mutable_json_text = strdup(example_json_text);

    jsish_decoder_t json;

    // Memory used by the decoder for storing decoded data.
    jsish_value_t json_values[1024];

    // Sets up the decoder to use the memory in json_values.
    jsish_init_decoder(&json, json_values, 1024);

    // Do the actual decoding.
    jsish_result_t result = jsish_decode(&json, mutable_json_text);

    // Check for errors.
    if (result == JSISH_ERR_MEM_OVERFLOW) {
        fprintf(stderr, "Out of memory at position %u\n", json.cursor);
        return 1;
    } else if (result == JSISH_ERR_MALFORMED) {
        fprintf(stderr, "Malformed JSON data at position %u\n", json.cursor);
        return 1;
    }

    // Iterate over the keys in the decoded object.
    
    jsish_value_t* node = &json.root;
    do {
        jsish_value_t* key = JSISH_KV_KEY(node);
        jsish_value_t* value = JSISH_KV_VALUE(node);

        if (JSISH_IS_ARRAY(value)) {
            jsish_value_t* array_element = JSISH_ARRAY_INDEX(value, 0);
            // ...
        }
    } while ((node = JSISH_KV_NEXT(node)) != NULL);

    return 0;
}
```

## Encoder usage

Warning: The encoder does not currently have any mechanism for reporting
errors. It will not overflow, but there is no way to detect if the output was
truncated. This will be addressed in an upcoming version.

```c
unsigned int jsish_encode(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size);
```

Encodes the data stored in `value` into `buffer`. The return value is the number
of bytes written to the buffer.

## API

See the section marked "Public API" in [the header file](jsish.h).

