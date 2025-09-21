/* JSISH - JSON Serialization in Single Header
 *
 * To include function definitions, define JSISH_MAIN before including this
 * header. This ensures that the definitions will exist in one compilation unit
 * only, reducing code bloat and the need to declare functions static.
 *
 * You can prevent inclusion of standard library headers by defining
 * JSISH_NO_STDLIB before including this header. In that case, you will need to
 * also define JSISH_STRTOD so it is an alias for an implementation of the
 * stdlib strtod() function, JSISH_SPRINTF as an alias for the sprintf()
 * function, JSISH_STRLEN as an alias for strlen(), JSISH_STRCMP for strcmp(),
 * and JSISH_FLOAT_DIGITS(V) so that when it's invoked as a function it returns
 * the number of digits required * to serialize the double precision floating
 * point number V.
 *
 * You can also define either of the above without defining JSISH_NO_STDLIB, in
 * which case they will override the default versions.
 *
 * =====
 *
 * zlib License
 * 
 * Copyright (c) 2025 Martin Evald
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.

 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:

 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#ifndef __JSISH_H
#define __JSISH_H

#define JSISH_VERSION_MAJOR 1
#define JSISH_VERSION_MINOR 1
#define JSISH_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif

#ifndef JSISH_NO_STDLIB
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef JSISH_STRTOD
#define JSISH_STRTOD strtod
#endif
#ifndef JSISH_SPRINTF
#define JSISH_SPRINTF sprintf
#endif
#ifndef JSISH_STRLEN
#define JSISH_STRLEN strlen
#endif
#ifndef JSISH_STRCMP
#define JSISH_STRCMP strcmp
#endif
#endif

#ifndef NULL
#define NULL 0
#endif

typedef enum {
	JSISH_OK = 0,
	JSISH_ERR_MALFORMED,
	JSISH_ERR_MEM_OVERFLOW
} jsish_result_t;

typedef enum {
	JSISH_NULL,
	JSISH_NUMBER,
	JSISH_BOOL,
	JSISH_STRING,
	JSISH_ARRAY,
	JSISH_KEYVAL
} jsish_type_t;

struct jsish_value;

typedef struct {
	unsigned int size;
	struct jsish_value* data;
} jsish_array_t;

typedef struct {
	struct jsish_value* key;
	struct jsish_value* value;
	struct jsish_value* next;
} jsish_keyval_t;

typedef struct jsish_value {
	jsish_type_t type;
	union {
		double vnum;
		char vbool;
		const char* vstr; 
		jsish_array_t varr;
		/* An object is just a list of key-value pairs. */
		jsish_keyval_t vobj;
	} data;
} jsish_value_t;

typedef struct {
	jsish_value_t* values;
	unsigned int values_cursor;
	unsigned int values_size;
	unsigned int stack_cursor;

	jsish_value_t root;

	char* source;
	unsigned int cursor;
} jsish_decoder_t;

/* Public API */

void jsish_init_decoder(
		jsish_decoder_t* decoder,
		jsish_value_t* values,
		unsigned int values_size);

jsish_result_t jsish_decode(jsish_decoder_t* decoder, char* source);

jsish_result_t jsish_encode(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes);

jsish_value_t* jsish_get_property(const jsish_value_t* value, const char* key);

#define JSISH_IS_NUMBER(VALUE) ((VALUE)->type == JSISH_NUMBER)
#define JSISH_IS_BOOL(VALUE) ((VALUE)->type == JSISH_BOOL)
#define JSISH_IS_STRING(VALUE) ((VALUE)->type == JSISH_STRING)
#define JSISH_IS_NULL(VALUE) ((VALUE)->type == JSISH_NULL)
#define JSISH_IS_ARRAY(VALUE) ((VALUE)->type == JSISH_ARRAY)
#define JSISH_IS_KEYVAL(VALUE) ((VALUE)->type == JSISH_KEYVAL)

#define JSISH_GET_NUMBER(VALUE) ((VALUE)->data.vnum)
#define JSISH_GET_BOOL(VALUE) ((VALUE)->data.vbool)
#define JSISH_GET_STRING(VALUE) ((VALUE)->data.vstr)

#define JSISH_ARRAY_INDEX(VALUE, INDEX) &((VALUE)->data.varr.data[INDEX])
#define JSISH_ARRAY_SIZE(VALUE) ((VALUE)->data.varr.size)

#define JSISH_KV_KEY(VALUE) ((VALUE)->data.vobj.key->data.vstr)
#define JSISH_KV_VALUE(VALUE) ((VALUE)->data.vobj.value)
#define JSISH_KV_NEXT(VALUE) ((VALUE)->data.vobj.next)

/* Function definitions below this line. */

#ifdef JSISH_MAIN

#ifndef JSISH_FLOAT_DIGITS
#if __STDC_VERSION__ >= 199901L
#define JSISH_FLOAT_DIGITS(V) snprintf(NULL, 0, "%g", (V))
#else
unsigned int _jsish_float_digits(double v) {
	static char buffer[1024];
	return JSISH_SPRINTF(buffer, "%g", v);
}
#define JSISH_FLOAT_DIGITS(V) _jsish_float_digits((V))
#endif
#endif

void jsish_init_decoder(
		jsish_decoder_t* decoder,
		jsish_value_t* values,
		unsigned int values_size) {
	decoder->values = values;
	decoder->values_size = values_size;
	decoder->values_cursor = 0;
	decoder->stack_cursor = values_size - 1;
	decoder->source = NULL;
	decoder->cursor = 0;
}

int _jsish_is_whitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void _jsish_skip_whitespace(jsish_decoder_t* decoder) {
	while (_jsish_is_whitespace(decoder->source[decoder->cursor])) {
		decoder->cursor++;
	}
}

jsish_value_t* _jsish_alloc_value(jsish_decoder_t* decoder) {
	jsish_value_t* value;
	if (decoder->values_cursor + 1 >= decoder->stack_cursor) {
		return NULL;
	}

	value = &decoder->values[decoder->values_cursor++];
	value->type = JSISH_NULL;
	value->data.vobj.key = NULL;
	value->data.vobj.value = NULL;
	value->data.vobj.next = NULL;

	return value;
}

jsish_value_t* _jsish_alloc_fifo(jsish_decoder_t* decoder) {
	jsish_value_t* stack_val;
	if (decoder->stack_cursor <= decoder->values_cursor + 1) {
		return NULL;
	}

	stack_val = &decoder->values[decoder->stack_cursor--];
	stack_val->type = JSISH_NULL;
	stack_val->data.vobj.key = NULL;
	stack_val->data.vobj.value = NULL;
	stack_val->data.vobj.next = NULL;

	return stack_val;
}

jsish_value_t* _jsish_fifo_copy(jsish_decoder_t* decoder, unsigned int index) {
	jsish_value_t* value;
	jsish_value_t* stack_val;
	if (decoder->values_size - decoder->stack_cursor - 1 < index) {
		return NULL;
	}

	value = _jsish_alloc_value(decoder);
	if (!value) {
		return NULL;
	}
	stack_val = &decoder->values[decoder->values_size - index - 1];
	value->type = stack_val->type;
	value->data = stack_val->data;

	return value;
}

int _jsish_is_hex_digit(char c) {
	return (c >= '0' && c <= '9')
		|| (c >= 'a' && c <= 'f')
		|| (c >= 'A' && c <= 'F');
}

jsish_result_t
_jsish_decode_string(jsish_decoder_t* decoder, jsish_value_t* value) {
	char c;
	int i;
	char* decoded; 
	if (decoder->source[decoder->cursor] != '"') {
		return JSISH_ERR_MALFORMED;
	}

	decoded = &decoder->source[decoder->cursor + 1];

	while ((c = decoder->source[++decoder->cursor]) != '"') {
		switch (c) {
			case '\\':
				/* Verify the backslash is followed by a valid escape code. */
				c = decoder->source[++decoder->cursor];
				switch (c) {
					case '\\': case '/': case '"': case 'f': case 't': case 'n':
					case 'r':
						break;
					case 'u':
						/* \u needs to be followed by four hex digits. */
						for (i = 0; i < 4; ++i) {
							c = decoder->source[++decoder->cursor];
							if (!_jsish_is_hex_digit(c)) {
								return JSISH_ERR_MALFORMED;
							}
						}
						break;
					default:
						return JSISH_ERR_MALFORMED;
				}
				break;
			case '\0': case '\n': case '\r':
				return JSISH_ERR_MALFORMED;
			default:
				break;
		}
	}

	/* Replace the end quote with a zero terminator in the source, so the
	 * decoded string can be referenced in situ. */
	decoder->source[decoder->cursor++] = '\0';
	value->type = JSISH_STRING;
	value->data.vstr = decoded;

	_jsish_skip_whitespace(decoder);

	return JSISH_OK;
}

jsish_result_t
_jsish_decode_number(jsish_decoder_t* decoder, jsish_value_t* value) {
	double num;
	char* end;
	num = JSISH_STRTOD(&decoder->source[decoder->cursor], &end);

	if (end == &decoder->source[decoder->cursor]) {
		return JSISH_ERR_MALFORMED;
	}

	value->type = JSISH_NUMBER;
	value->data.vnum = num;
	decoder->cursor += (unsigned int) (end - &decoder->source[decoder->cursor]);

	_jsish_skip_whitespace(decoder);

	return JSISH_OK;
}

jsish_result_t
_jsish_decode_bool(jsish_decoder_t* decoder, jsish_value_t* value) {
	const char* s;
	s = &decoder->source[decoder->cursor];
	if (s[0] == 'f' 
			&& s[1] == 'a'
			&& s[2] == 'l'
			&& s[3] == 's'
			&& s[4] == 'e') {
		value->data.vbool = 0;
		decoder->cursor += 5;
	} else if (s[0] == 't' && s[1] == 'r' && s[2] == 'u' && s[3] == 'e') {
		value->data.vbool = 1;
		decoder->cursor += 4;
	} else {
		return JSISH_ERR_MALFORMED;
	}
	value->type = JSISH_BOOL;

	_jsish_skip_whitespace(decoder);

	return JSISH_OK;
}

jsish_result_t
_jsish_decode_null(jsish_decoder_t* decoder, jsish_value_t* value) {
	const char* s;
	s = &decoder->source[decoder->cursor];
	/* Skip checking the first char since that's already been done in
	 * _jsish_decode_value(). */
	if (s[1] != 'u' || s[2] != 'l' || s[3] != 'l') {
		return JSISH_ERR_MALFORMED;
	}
	value->type = JSISH_NULL;
	decoder->cursor += 4;

	_jsish_skip_whitespace(decoder);

	return JSISH_OK;
}

jsish_result_t
_jsish_decode_value(jsish_decoder_t* decoder, jsish_value_t* value);

jsish_result_t
_jsish_decode_array(jsish_decoder_t* decoder, jsish_value_t* value) {
	char c;
	int first;
	unsigned int i;
	jsish_value_t* element;
	jsish_result_t result;
	first = 1;

	/* Skip over initial brace. */
	decoder->cursor++;
	_jsish_skip_whitespace(decoder);

	/* Decode all the array elements and push the values onto a FIFO stack
	 * at the top of the provided values memory. */
	while ((c = decoder->source[decoder->cursor]) != ']') {
		if (!first && c != ',') {
			return JSISH_ERR_MALFORMED;
		} else if (first) {
			first = 0;
		} else { /* c == ',' -- Skip over comma. */
			decoder->cursor++;
		}

		element = _jsish_alloc_fifo(decoder);
		if (!element) {
			return JSISH_ERR_MEM_OVERFLOW;
		}
		
		_jsish_skip_whitespace(decoder);
		result = _jsish_decode_value(decoder, element);
		if (result != JSISH_OK) {
			return result;
		}

		_jsish_skip_whitespace(decoder);

		value->data.varr.size++;
	}

	/* Read array elements in FIFO order from the stack, copy them so they are
	 * contiguous, and reset stack cursor. */
	element = NULL;
	for (i = 0; i < value->data.varr.size; ++i) {
		element = _jsish_fifo_copy(decoder, i);
		if (!element) {
			return JSISH_ERR_MEM_OVERFLOW;
		}
		if (!value->data.varr.data) {
			/* First element in array. */
			value->data.varr.data = element;
		}
	}
	decoder->stack_cursor = decoder->values_size - 1;
	value->type = JSISH_ARRAY;

	/* Account for the closing bracket. */
	decoder->cursor++;
	_jsish_skip_whitespace(decoder);

	return JSISH_OK;
}

jsish_result_t
_jsish_decode_object(jsish_decoder_t* decoder, jsish_value_t* value) {
	int first;
	char c;
	jsish_result_t result;
	if (decoder->source[decoder->cursor] != '{') {
		return JSISH_ERR_MALFORMED;
	}
	value->type = JSISH_KEYVAL;
	/* Decode fields in object. */
	decoder->cursor++;
	_jsish_skip_whitespace(decoder);
	first = 1;
	while ((c = decoder->source[decoder->cursor]) != '}') {
		if (!first && c == ',') {
			value->data.vobj.next = _jsish_alloc_value(decoder);
			if (!value->data.vobj.next) {
				return JSISH_ERR_MEM_OVERFLOW;
			}
			value = value->data.vobj.next;
			value->type = JSISH_KEYVAL;
			decoder->cursor++;
		} else if (!first) {
			return JSISH_ERR_MALFORMED;
		} else {
			first = 0;
		}

		/* Decode key. */
		value->data.vobj.key = _jsish_alloc_value(decoder);
		if (!value->data.vobj.key) {
			return JSISH_ERR_MEM_OVERFLOW;
		}
		_jsish_skip_whitespace(decoder);
		result = _jsish_decode_string(decoder, value->data.vobj.key);
		if (result != JSISH_OK) {
			return result;
		}

		/* Verify key is followed by colon, advance cursor, and decode value. */
		_jsish_skip_whitespace(decoder);
		if (decoder->source[decoder->cursor] == ':') {
			decoder->cursor++;
		} else {
			return JSISH_ERR_MALFORMED;
		}
		_jsish_skip_whitespace(decoder);

		value->data.vobj.value = _jsish_alloc_value(decoder);
		if (!value->data.vobj.value) {
			return JSISH_ERR_MEM_OVERFLOW;
		}
		result = _jsish_decode_value(decoder, value->data.vobj.value);
		if (result != JSISH_OK) {
			return result;
		}
	}

	/* Account for the closing brace. */
	decoder->cursor++;

	_jsish_skip_whitespace(decoder);

	return JSISH_OK;
}

jsish_result_t
_jsish_decode_value(jsish_decoder_t* decoder, jsish_value_t* value) {
	char c = decoder->source[decoder->cursor];
	switch (c) {
		case '"':
			return _jsish_decode_string(decoder, value);
		case '-': case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			return _jsish_decode_number(decoder, value);
		case 't': case 'f':
			return _jsish_decode_bool(decoder, value);
		case 'n':
			return _jsish_decode_null(decoder, value);
		case '[':
			return _jsish_decode_array(decoder, value);
		case '{':
			return _jsish_decode_object(decoder, value);
		default:
			return JSISH_ERR_MALFORMED;
	};
}

jsish_result_t jsish_decode(jsish_decoder_t* decoder, char* source) {
	decoder->source = source;
	_jsish_skip_whitespace(decoder);
	return _jsish_decode_value(decoder, &decoder->root);
}

void _jsish_append(
		char* buffer,
		char c,
		unsigned int buffer_size,
		unsigned int* encoded_bytes) {
	if (*encoded_bytes < buffer_size) {
		buffer[*encoded_bytes] = c;
	}
	(*encoded_bytes)++;
}

void _jsish_encode_null(
		char* buffer, unsigned int buffer_size, unsigned int* encoded_bytes) {
	_jsish_append(buffer, 'n', buffer_size, encoded_bytes); 
	_jsish_append(buffer, 'u', buffer_size, encoded_bytes); 
	_jsish_append(buffer, 'l', buffer_size, encoded_bytes); 
	_jsish_append(buffer, 'l', buffer_size, encoded_bytes); 
}

void _jsish_encode_number(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes) {
	unsigned int length;
	length = JSISH_FLOAT_DIGITS(JSISH_GET_NUMBER(value));
	if (*encoded_bytes + length <= buffer_size) {
		JSISH_SPRINTF(&buffer[*encoded_bytes], "%g", JSISH_GET_NUMBER(value));
	}
	*encoded_bytes += length;
}

void _jsish_encode_bool(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes) {
	if (JSISH_GET_BOOL(value)) {
		_jsish_append(buffer, 't', buffer_size, encoded_bytes); 
		_jsish_append(buffer, 'r', buffer_size, encoded_bytes); 
		_jsish_append(buffer, 'u', buffer_size, encoded_bytes); 
		_jsish_append(buffer, 'e', buffer_size, encoded_bytes); 
		return;
	}
	_jsish_append(buffer, 'f', buffer_size, encoded_bytes); 
	_jsish_append(buffer, 'a', buffer_size, encoded_bytes); 
	_jsish_append(buffer, 'l', buffer_size, encoded_bytes); 
	_jsish_append(buffer, 's', buffer_size, encoded_bytes); 
	_jsish_append(buffer, 'e', buffer_size, encoded_bytes); 
}

void _jsish_encode_string(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes) {
	unsigned int length;
	/* String length + quotation marks. */
	length = JSISH_STRLEN(JSISH_GET_STRING(value)) + 2;
	if (*encoded_bytes + length <= buffer_size) { 
		JSISH_SPRINTF(
				&buffer[*encoded_bytes], "\"%s\"", JSISH_GET_STRING(value));
	}
	*encoded_bytes += length;
}

void _jsish_encode_value(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes);

void _jsish_encode_array(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes) {
	unsigned int i;
	int sep;
	_jsish_append(buffer, '[', buffer_size, encoded_bytes); 
	sep = 0;
	for (i = 0; i < JSISH_ARRAY_SIZE(value); ++i) {
		/* Write the field separator. */
		if (sep) {
			_jsish_append(buffer, ',', buffer_size, encoded_bytes); 
		}

		/* Encode the value at array index i.  */
		_jsish_encode_value(
				JSISH_ARRAY_INDEX(value, i),
				buffer,
				buffer_size,
				encoded_bytes);

		sep = 1;
	}
	_jsish_append(buffer, ']', buffer_size, encoded_bytes); 
}

void _jsish_encode_object(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes) {
	int sep;
	_jsish_append(buffer, '{', buffer_size, encoded_bytes);
	sep = 0;
	while (value != NULL) {
		/* Write the field separator. */
		if (sep) {
			_jsish_append(buffer, ',', buffer_size, encoded_bytes); 
		}

		/* Encode the property name/key. */
		_jsish_encode_string(
				value->data.vobj.key,
				buffer,
				buffer_size,
				encoded_bytes);
		_jsish_append(buffer, ':', buffer_size, encoded_bytes);

		/* Encode the value. */
		_jsish_encode_value(
				JSISH_KV_VALUE(value),
				buffer,
				buffer_size,
				encoded_bytes);

		sep = 1;
		value = JSISH_KV_NEXT(value);
	}
	_jsish_append(buffer, '}', buffer_size, encoded_bytes);
}

void _jsish_encode_value(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes) {
	switch (value->type) {
		case JSISH_NULL:
			_jsish_encode_null(buffer, buffer_size, encoded_bytes);
			return;
		case JSISH_NUMBER:
			_jsish_encode_number(value, buffer, buffer_size, encoded_bytes);
			return;
		case JSISH_BOOL:
			_jsish_encode_bool(value, buffer, buffer_size, encoded_bytes);
			return;
		case JSISH_STRING:
			_jsish_encode_string(value, buffer, buffer_size, encoded_bytes);
			return;
		case JSISH_ARRAY:
			_jsish_encode_array(value, buffer, buffer_size, encoded_bytes);
			return;
		case JSISH_KEYVAL:
			_jsish_encode_object(value, buffer, buffer_size, encoded_bytes);
			return;
		default:
			return;
	}
}

jsish_result_t jsish_encode(
		const jsish_value_t* value,
		char* buffer,
		unsigned int buffer_size,
		unsigned int* encoded_bytes) {
	*encoded_bytes = 0;
	_jsish_encode_value(value, buffer, buffer_size, encoded_bytes);
	_jsish_append(buffer, '\0', buffer_size, encoded_bytes);

	return *encoded_bytes <= buffer_size ? JSISH_OK : JSISH_ERR_MEM_OVERFLOW;
}

jsish_value_t* jsish_get_property(const jsish_value_t* value, const char* key) {
	do {
		if (JSISH_STRCMP(JSISH_KV_KEY(value), key) == 0) {
			return JSISH_KV_VALUE(value);
		}
	} while ((value = JSISH_KV_NEXT(value)) != NULL);

	return NULL;
}

#endif

#ifdef __cplusplus
}
#endif

#endif

