// jsonic.h : Include file for standard system include files,
// or project specific include files.

#ifndef JSONIC_H
#define JSONIC_H

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
    const char JSONIC_OBJECT;
    const char JSONIC_ARRAY;
    const char JSONIC_STRING;
    const char JSONIC_NUMBER;
    const char JSONIC_BOOLEAN;
    const char JSONIC_NAMED_OBJECT;
    const char JSONIC_NAMED_ARRAY;
    const char JSONIC_OBJECT_END;
    const char JSONIC_ARRAY_END;
    const char JSONIC_UNQUOTED_VALUE;

    /**
     *  Enum for possible causes of error
     */
    typedef enum {
        JSONIC_NO_ERROR = 0,
        JSONIC_ERROR_INVALID_INPUT = 1,
        JSONIC_ERROR_MEM_ALLOC = 2,
        JSONIC_ERROR_NO_DATA = 3,
        JSONIC_ERROR_STACK_OVERFLOW
    } jsonic_error_t;

    /**
     *  Enum to handle serialization format
     */
    typedef enum {
        JSONIC_PRETTY_PRINT,
        JSONIC_COMPACT_PRINT
    } jsonic_print_t;

    typedef struct {
        short type;
        size_t key_start;
        size_t key_end;
        size_t value_start;
        size_t value_end;
    } jsonic_element_t;

    extern uint MALLOCS;
    extern uint FREES;

    /**************************/
    /*                        */
    /*     User functions     */
    /*                        */
    /**************************/

    /**
     * Always start by calling jsonic_init() whether you are serializing or deserializing.
     * Returns zero on success or non-zero on failure. The return value is the same as returned by the function jsonic_get_last_error().
     */
    uint jsonic_init();

    /**
     * Always call jsonic_free() when done.
     */
    void jsonic_free();

    /**
     * Parse a Json string
     *
     * Returns a pointer to the root element of the structure.
     * If pointer value is zero, call jsonic_get_last_error() for error code.
     */
    void jsonic_parse(char* json);

    /**
     * Serialize a Json-structure to a string.
     *
     * Parameters:
     *      root:        If you have parsed or created a new structure and want to serialize the whole structure, you can pass NULL here. Its purpose is to be able to serialize part of a structure.
     *      print_type:  Use a value from the jsonic_print_t struct, either JSONIC_COMPACT_PRINT (no spaces or line breaks) or JSONIC_PRETTY_PRINT (formatted). If NULL is passed, JSONIC_PRETTY_PRINT is assumed.
     *      output_size: The size in bytes of the string returned. If you don't care about the size you can pass NULL here.
     *
     * Returns a pointer to a string containing the serialized data.
     */
    char* jsonic_serialize(jsonic_element_t* root_element, jsonic_print_t print_type, uint* output_size);

    /**
     * Call this method to get the latest error code.
     */
    jsonic_error_t jsonic_get_last_error();

    /**
     * Call this method to get the position where the parse-error occured. Note that the position includes count of 'binary bytes' like new line, tabs etc.
     * Returns negative value if no error has occured.
     */
    int jsonic_get_error_pos();


#ifdef __cplusplus
}
#endif

#endif //JSONIC_H