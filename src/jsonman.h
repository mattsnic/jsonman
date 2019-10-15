// jsonman.h : Include file for standard system include files,
// or project specific include files.

#ifndef JSONMAN_H
#define JSONMAN_H

#define log(m) printf("%s\n", m)
#define STACK_SIZE 100

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef unsigned int uint;

    /**
     * Type of value
     */
    const char JSONMAN_OBJECT;
    const char JSONMAN_ARRAY;
    const char JSONMAN_STRING;
    const char JSONMAN_NUMBER;
    const char JSONMAN_BOOLEAN;
    const char JSONMAN_NAMED_OBJECT;
    const char JSONMAN_NAMED_ARRAY;
    const char JSONMAN_OBJECT_END;
    const char JSONMAN_ARRAY_END;
    const char JSONMAN_UNQUOTED_VALUE;

    /**
     *  Enum for possible causes of error
     */
    typedef enum {
        JSONMAN_NO_ERROR = 0,
        JSONMAN_ERROR_INVALID_INPUT = 1,
        JSONMAN_ERROR_MEM_ALLOC = 2,
        JSONMAN_ERROR_NO_DATA = 3,
        JSONMAN_ERROR_STACK_OVERFLOW
    } jsonman_error_t;

    /**
     *  Enum to handle serialization format
     */
    typedef enum {
        JSONMAN_PRETTY_PRINT,
        JSONMAN_COMPACT_PRINT
    } jsonman_print_t;

    typedef struct {
        short type;
        size_t key_start;
        size_t key_end;
        size_t value_start;
        size_t value_end;
    } jsonman_element_t;

    extern uint MALLOCS;
    extern uint FREES;

    /**************************/
    /*                        */
    /*     User functions     */
    /*                        */
    /**************************/

    /**
     * Always start by calling jsonman_init() whether you are serializing or deserializing.
     * Returns zero on success or non-zero on failure. The return value is the same as returned by the function jsonman_get_last_error().
     */
    uint jsonman_init();

    /**
     * Always call jsonman_free() when done.
     */
    void jsonman_free();

    /**
     * Parse a Json string
     *
     * Returns a pointer to the root element of the structure.
     * If pointer value is zero, call jsonman_get_last_error() for error code.
     */
    void jsonman_parse(char* json);

    /**
     * Serialize a Json-structure to a string.
     *
     * Parameters:
     *      root:        If you have parsed or created a new structure and want to serialize the whole structure, you can pass NULL here. Its purpose is to be able to serialize part of a structure.
     *      print_type:  Use a value from the jsonman_print_t struct, either JSONMAN_COMPACT_PRINT (no spaces or line breaks) or JSONMAN_PRETTY_PRINT (formatted). If NULL is passed, JSONMAN_PRETTY_PRINT is assumed.
     *      output_size: The size in bytes of the string returned. If you don't care about the size you can pass NULL here.
     *
     * Returns a pointer to a string containing the serialized data.
     */
    char* jsonman_serialize(jsonman_element_t* root_element, jsonman_print_t print_type, uint* output_size);

    /**
     * Call this method to get the latest error code.
     */
    jsonman_error_t jsonman_get_last_error();

    /**
     * Call this method to get the position where the parse-error occured. Note that the position includes count of 'binary bytes' like new line, tabs etc.
     * Returns negative value if no error has occured.
     */
    int jsonman_get_error_pos();


#ifdef __cplusplus
}
#endif

#endif //JSONMAN_H