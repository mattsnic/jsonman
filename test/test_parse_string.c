#include "test.h"

int main()
{
    /*
     * Testing with one string
     */
    char* one_string = "{                            \
                            \"key1\" : \"value1\"    \
                        }";

    jsonman_parse(one_string);
    jsonman_free();

    if (MALLOCS != FREES)
    {
        ERROR;
        return 1;
    }

    if (!mem_alloc_ok())
    {
        ERROR;
        return 1;
    }

    if (error_code())
    {
        ERROR;
        return 1;
    }


    /*
     * Testing with two strings
     */
    char* two_strings = "{                            \
                            \"key1\" : \"value1\",    \
                            \"key2\" : \"value2\"     \
                         }";

    jsonman_parse(two_strings);
    jsonman_free();

    if (!mem_alloc_ok())
    {
        ERROR;
        return 1;
    }

    if (error_code())
    {
        ERROR;
        return 1;
    }

    return 0;
}

