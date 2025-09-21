#define JSISH_MAIN
#include <jsish.h>

#include <stdio.h>

#define MAX_FILE_SIZE 16384
#define VALUE_MEM_SIZE 8192

int main(int argc, char** argv) {
	size_t length;
	unsigned int buffer_size;
	jsish_result_t json_result;
	jsish_decoder_t json;
	jsish_value_t json_values[VALUE_MEM_SIZE];
	char json_text[MAX_FILE_SIZE];
	char* output;

	length = fread(json_text, 1, MAX_FILE_SIZE, stdin);
    if (ferror(stdin) != 0) {
        fputs("Error while reading from standard input\n", stderr);
		return 1;
    }
	json_text[length] = '\0';

	jsish_init_decoder(&json, json_values, VALUE_MEM_SIZE);
	
	json_result = jsish_decode(&json, json_text);
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

	json_result = jsish_encode(&json.root, NULL, 0, &buffer_size);
	if (json_result != JSISH_ERR_MEM_OVERFLOW) {
		fprintf(stderr, "Error: Encoded JSON is zero bytes long.\n");
		return 4;
	}
	output = malloc(buffer_size);
	if (!output) {
		fprintf(stderr, "Failed to allocate memory for encoding.\n");
		return 5;
	}
	json_result = jsish_encode(&json.root, output, buffer_size, &buffer_size);
	fprintf(stdout, "Re-encoded data: %s\n", output);

	return 0;
}

