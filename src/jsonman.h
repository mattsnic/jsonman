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
        JSONMAN_ERROR_STACK_OVERFLOW = 4,
        JSONMAN_ERROR_INVALID_ID = 5,
        JSONMAN_ERROR_SIMPLE_VALUE_NOT_PRESENT = 6,
        JSONMAN_ERROR_OUT_PARAMETER_IS_NULL = 7
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

    /*****************************/
    /*                           */
    /*     General functions     */
    /*                           */
    /*****************************/

    /**
     * Always call jsonman_free() when done.
     */
    void jsonman_free();

    /**
     * Call this method to get the latest error code.
     */
    jsonman_error_t jsonman_get_last_error();

    /**
     * Call this method to get the position where the parse-error occured. Note that the position includes count of 'binary bytes' like new line, tabs etc.
     * Returns negative value if no error has occured.
     */
    int jsonman_get_error_pos();



    /**********************************/
    /*                                */
    /*          Json parsing          */
    /*                                */
    /**********************************/

    /**
     * Parse a Json string
     *
     * Returns zero on success. If value is non-zero, call jsonman_get_last_error() to get the reason.
     */
    int jsonman_parse(char* json);


    /***************************************/
    /*                                     */
    /*          Json manipulation          */
    /*                                     */
    /***************************************/

    /*
     * Returns the key length for a particular element id in the size_t out parameter.
     * Return value from the function is zero on success and non-zero on failure. Call function jsonman_get_last_error() for reason.

     */
    int get_key_length(int id, size_t* out_value);

    /*
     * Returns the value length for a particular element id in the size_t out parameter. This assumes that the id points to an element of either a string, number, boolean or an unquoted value (not object, array etc.)
     * Return value from the function is zero on success and non-zero on failure. Call function jsonman_get_last_error() for reason.
     */
    int get_value_length(int id, size_t* out_value);

    /*
     * Get the key for a given id as a string if a key is present. Use get_key_length() function to determine size to allocate for the buffer (the size does not include the null-terminator).
     * Returns zero on success and non-zero on failure. Call function jsonman_get_last_error() for reason.
     */
    int get_key(int id, char* out_buffer);

    /*
     * Returns the value for a given id as a string value in the output buffer. 
     * Returns zero on success and non-zero on failure. Call function jsonman_get_last_error() for reason.
     */
    int get_value_as_string(int id, char* out_buffer);

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



    /*************************************************************************************/
    /*                                                                                   */
    /*      Functions used by tests that are probably not very useful in other cases     */
    /*                                                                                   */
    /*************************************************************************************/

     /*
      * Convenience function to get the next element id. Root element id is always zero.
      * If no root element exist a negative value is returned.
      */
    int next_id(int id);

    /*
     * Get element type for the given id. If given id is invalid, a negative number is returned.
     */
    short get_type(int id);



#ifdef __cplusplus
}
#endif

#endif //JSONMAN_H
