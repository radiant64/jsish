/* JSISH - JSON Serialization in Single Header
 *
 * =====
 *
 * zlib license:
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

#ifndef JSISH_NO_MALLOC
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	JSISH_OK = 0,
	JSISH_ERR_PARSER,
	JSISH_ERR_MEMORY
} jsish_result_t;

typedef enum {
	JSISH_NULL,
	JSISH_NUMBER,
	JSISH_BOOL,
	JSISH_STRING,
	JSISH_ARRAY,
	JSISH_STRING
} jsish_type_t;

struct jsish_value;

typedef struct {
	unsigned int size;
	struct jsish_value* data;
} jsish_array_t;

typedef struct jsish_value {
	jsish_type_t type;
	union {
		double vnum;
		char vbool;
		const char* vstr; 
		jsish_array_t varr;
		struct jsish_value* vobj;
	} data;
} jsish_value_t;

typedef struct {
	jsish_value_t* values;
	unsigned int values_cursor;
	unsigned int values_size;

	const char* source;
	unsigned int cursor;
} jsish_parser_t;

void jsish_init_parser(
		jsish_value_t* values,
		unsigned int values_size,
		jsish_parser_t* parser) {
	parser->values = values;
	parser->values_size = values_size;
	parser->values_cursor = 0;
	parser->cursor = 0;
}

#ifdef __cplusplus
}
#endif

#endif

