/* JSISH - JSON Serialization in Single Header
 *
 * To include function definitions, define JSISH_MAIN before including this
 * header. This ensures that the definitions will exist in one compilation unit
 * only, reducing code bloat and the need to declare functions static.
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
#define JSISH_VERSION_MINOR 0
#define JSISH_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

typedef enum {
	JSISH_OK,
	JSISH_ERR_MALFORMED,
	JSISH_ERR_MEM_OVERFLOW
} jsish_result_t;

typedef enum {
	JSISH_NULL,
	JSISH_NUMBER,
	JSISH_BOOL,
	JSISH_STRING,
	JSISH_ARRAY,
	JSISH_STRING,
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
		jsish_keyval_t vobj;
	} data;
} jsish_value_t;

typedef struct {
	jsish_value_t* values;
	unsigned int values_cursor;
	unsigned int values_size;

	jsish_keyval_t root;

	char* source;
	unsigned int cursor;
} jsish_decoder_t;

/* Public API */

void jsish_init_decoder(
		jsish_decoder_t* decoder,
		jsish_value_t* values,
		unsigned int values_size);

jsish_result_t jsish_decode(jsish_decoder_t* decoder, char* source);

/* Function definitions below this line. */

#ifdef JSISH_MAIN

void jsish_init_decoder(
		jsish_decoder_t* decoder,
		jsish_value_t* values,
		unsigned int values_size) {
	decoder->values = values;
	decoder->values_size = values_size;
	decoder->values_cursor = 0;
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
	if (decoder->values_cursor + 1 => decoder->values_size) {
		return NULL;
	}

	value = &decoder->values[decoder->values_cursor++];
	value->type = JSISH_NULL;
	value->data.vobj.key = NULL;
	value->data.vobj.value = NULL;
	value->data.vobj.next = NULL;

	return value;
}

jsish_result_t
_jsish_decode_object(jsish_decoder_t* decoder, jsish_keyval_t* keyval) {
	int first;
	char c;
	jsish_result_t result;
	if (decoder->source[decoder->cursor] != '{') {
		return JSISH_ERR_MALFORMED;
	}
	/* Decode fields in object. */
	decoder->cursor++;
	_jsish_skip_whitespace(decoder);
	first = 1;
	while ((c = decoder->source[decoder->cursor]) != '}') {
		if (!first && c == ',') {
			keyval->next = _jsish_alloc_value(decoder);
			if (!keyval->next) {
				return JSISH_ERR_MEM_OVERFLOW;
			}
			keyval = &keyval->next->data.vobj;
			keyval->type = JSISH_KEYVAL;
			decoder->cursor++;
		} else if (!first) {
			return JSISH_ERR_MALFORMED;
		} else {
			first = 0;
		}

		/* Decode key. */
		keyval->key = _jsish_alloc_value(decoder);
		if (!keyval->key) {
			return JSISH_ERR_MEM_OVERFLOW;
		}
		result = _jsish_decode_string(decoder, &keyval->key);
		if (result != JSISH_OK) {
			return result;
		}

		/* Verify key is followed by colon, advance cursor and decode value. */
		_jsish_skip_whitespace(decoder);
		if (decoder->source[decoder->cursor] == ':') {
			decoder->cursor++;
		} else {
			return JSISH_ERR_MALFORMED;
		}
		_jsish_skip_whitespace(decoder);

		keyval->value = _jsish_alloc_value(decoder);
		if (!keyval->value) {
			return JSISH_ERR_MEM_OVERFLOW;
		}
		result = _jsish_decode_value(decoder, &keyval->value);
		if (result != JSISH_OK) {
			return result;
		}
	}

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

jsish_result_t jsish_decode(jsish_decoder_t* decoder, const char* source) {
	decoder->source = source;
	_jsish_skip_whitespace(decoder);
	return _jsish_decode_value(jsish_decoder_t* decoder, &decoder->root);
}

#endif

#ifdef __cplusplus
}
#endif

#endif

