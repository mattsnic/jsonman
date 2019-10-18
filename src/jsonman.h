// jsonman.h : Include file for standard system include files,
// or project specific include files.

#ifndef JSONMAN_H
#define JSONMAN_H

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
    const char JM_OBJECT;
    const char JM_ARRAY;
    const char JM_STRING;
    const char JM_NUMBER;
    const char JM_BOOLEAN;
    const char JM_NAMED_OBJECT;
    const char JM_NAMED_ARRAY;
    const char JM_OBJECT_END;
    const char JM_ARRAY_END;
    const char JM_UNQUOTED_VALUE;

    /**
     *  Enum for possible causes of error
     */
    typedef enum {
        JM_NO_ERROR                       = 0,
        JM_ERROR_INVALID_INPUT            = 1,
        JM_ERROR_MEM_ALLOC                = 2,
        JM_ERROR_NO_DATA                  = 3,
        JM_ERROR_STACK_OVERFLOW           = 4,
        JM_ERROR_INVALID_ID               = 5,
        JM_ERROR_SIMPLE_VALUE_NOT_PRESENT = 6,
        JM_ERROR_OUT_PARAMETER_IS_NULL    = 7,
        JM_ERROR_ELEMENT_NOT_FOUND        = 8
    } jm_error_t;

    /**
     *  Enum to handle serialization format
     */
    typedef enum {
        JM_PRETTY,
        JM_COMPACT
    } jm_format_t;

    typedef struct {
        short type;
        int level;
        size_t key_start;
        size_t key_end;
        size_t value_start;
        size_t value_end;
    } jm_element_t;

    extern uint MALLOCS;
    extern uint FREES;

    /*****************************/
    /*                           */
    /*     General functions     */
    /*                           */
    /*****************************/

    /**
     * Always call jm_free() when done.
     */
    void jm_free();

    /**
     * Call this method to get the latest error code.
     */
    jm_error_t jm_get_last_error();

    /**
     * Call this method to get the position where the parse-error occured. Note that the position includes count of 'binary bytes' like new line, tabs etc.
     * Returns negative value if no error has occured.
     */
    int jm_get_error_pos();



    /**********************************/
    /*                                */
    /*          Json parsing          */
    /*                                */
    /**********************************/

    /**
     * Parse a Json string
     *
     * Returns zero on success. If value is non-zero, call jm_get_last_error() to get the reason.
     */
    int jm_parse(char* json);


    /***************************************/
    /*                                     */
    /*          Json manipulation          */
    /*                                     */
    /***************************************/

    /*
     * Returns the key length for a particular element id in the size_t out parameter.
     * Return value from the function is zero on success and non-zero on failure. Call function jm_get_last_error() for reason.

     */
    int jm_get_key_length(int id, size_t* out_value);

    /*
     * Returns the value length for a particular element id in the size_t out parameter. This assumes that the id points to an element of either a string, number, boolean or an unquoted value (not object, array etc.)
     * Return value from the function is zero on success and non-zero on failure. Call function jm_get_last_error() for reason.
     */
    int jm_get_value_length(int id, size_t* out_value);

    /*
     * Get the key for a given id as a string if a key is present. Use jm_get_key_length() function to determine size to allocate for the buffer (the size does not include the null-terminator).
     * Returns zero on success and non-zero on failure. Call function jm_get_last_error() for reason.
     */
    int jm_get_key(int id, char* out_buffer);

    /*
     * Returns the value for a given id as a string value in the output buffer.
     * Returns zero on success and non-zero on failure. Call function jm_get_last_error() for reason.
     */
    int jm_get_value_as_string(int id, char* out_buffer);

    int jm_find_next_object(int from_id, int level);
    int jm_find_next_named_object(int from_id, int level, char* name);
    int jm_find_next_array(int from_id, int level);
    int jm_find_next_named_array(int from_id, int level, char* name);
    int jm_find_next_number(int from_id, int level, char* name, char* value);
    int jm_find_next_boolean(int from_id, int level, char* name, int* value);
    int jm_find_next_string(int from_id, int level, char* name, char* value);

    /**
     * Serialize a Json-structure to a string.
     *
     * Parameters:
     *      root:        If you have parsed or created a new structure and want to serialize the whole structure, you can pass NULL here. Its purpose is to be able to serialize part of a structure.
     *      print_type:  Use a value from the jm_format_t struct, either JM_COMPACT (no spaces or line breaks) or JM_PRETTY (formatted). If NULL is passed, JM_PRETTY is assumed.
     *      output_size: The size in bytes of the string returned. If you don't care about the size you can pass NULL here.
     *
     * Returns a pointer to a string containing the serialized data.
     */
    char* jm_serialize(jm_element_t* root_element, jm_format_t print_type, uint* output_size);



    /*************************************************************************************/
    /*                                                                                   */
    /*      Functions used by tests that are probably not very useful in other cases     */
    /*                                                                                   */
    /*************************************************************************************/

     /*
      * Convenience function to get the next element id. Root element id is always zero.
      * If no root element exist a negative value is returned.
      */
    int jm_next_id(int id);

    /*
     * Get element type for the given id. If given id is invalid, a negative number is returned.
     */
    short jm_get_type(int id);



#ifdef __cplusplus
}
#endif

#endif //JSONMAN_H
