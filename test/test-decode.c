#define JSISH_MAIN
#include <jsish.h>

#include <stdio.h>

#define MAX_FILE_SIZE 16384
#define VALUE_MEM_SIZE 8192

void print_value(const jsish_value_t* value, int indent);

void print_indent(int indent) {
	int i;

	for (i = 0; i < indent; ++i) {
		fputs("  ", stdout);
	}
}

void print_null() {
	fputs("null", stdout);
}

void print_number(const jsish_value_t* value) {
	fprintf(stdout, "%f", JSISH_GET_NUMBER(value));
}

void print_bool(const jsish_value_t* value) {
	fprintf(stdout, "%s", JSISH_GET_BOOL(value) ? "true" : "false");
}

void print_string(const jsish_value_t* value) {
	fprintf(stdout, "\"%s\"", JSISH_GET_STRING(value));
}

void print_array(const jsish_value_t* value, int indent) {
	int i;
	const char* sep;
	sep = "";
	for (i = 0; i < JSISH_ARRAY_SIZE(value); ++i) {
		fputs(sep, stdout);
		print_value(JSISH_ARRAY_INDEX(value, i), indent);
		sep = ", ";
	}
}

void print_object(const jsish_value_t* value, int indent) {
	fprintf(stdout, "%s:\n", JSISH_KV_KEY(value));
	print_indent(indent + 1);
	print_value(JSISH_KV_VALUE(value), indent + 1);
	fputs("\n", stdout);

	while ((value = JSISH_KV_NEXT(value)) != NULL) {
		print_object(value, indent);
	}
}

void print_value(const jsish_value_t* value, int indent) {
	switch (value->type) {
		case JSISH_NULL: print_null(); return;
		case JSISH_NUMBER: print_number(value); return;
		case JSISH_BOOL: print_bool(value); return;
		case JSISH_STRING: print_string(value); return;
		case JSISH_ARRAY: print_array(value, indent); return;
		case JSISH_KEYVAL: print_object(value, indent); return;
		default: return;
	}
}

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

	fputs("\n", stdout);
	print_value(&json.root, 0);

	return 0;
}

