#define JSISH_MAIN
#include <jsish.h>

#include <stdio.h>

#define MAX_FILE_SIZE 16384
#define VALUE_MEM_SIZE 8192

int main(int argc, char** argv) {
	size_t length;
	jsish_result_t json_result;
	jsish_decoder_t json;
	jsish_value_t json_values[VALUE_MEM_SIZE];
	char test_valid[MAX_FILE_SIZE];

	length = fread(test_valid, 1, MAX_FILE_SIZE, stdin);
    if (ferror(stdin) != 0) {
        fputs("Error while reading from standard input\n", stderr);
		return 1;
    }
	test_valid[length] = '\0';

	jsish_init_decoder(&json, json_values, VALUE_MEM_SIZE);
	
	json_result = jsish_decode(&json, test_valid);
	switch (json_result) {
		case JSISH_OK:
			fputs("Decoding succeeded.\n", stdout);
			break;
		case JSISH_ERR_MALFORMED:
			fprintf(stderr,
					"Decoding failed, malformed JSON data at position %u\n",
					json.cursor);
			return 2;
		case JSISH_ERR_MEM_OVERFLOW:
			fprintf(stderr,
					"Decoding failed, out of memory at position %u\n",
					json.cursor);
			return 3;
		default:
			fprintf(stderr,
					"Unhandled return code from jsish_decode(): %u\n",
					json_result);
	}

	return 0;
}

