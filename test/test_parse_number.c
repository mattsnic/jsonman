#include "test.h"

int main()
{
    /*
     * Testing with one number
     */
    char* single_string = "{                         \
                                \"int_nr\" : 5       \
                           }";

    jsonman_parse(single_string);
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
     * Testing with two numbers
     */
    char* two_strings = "{                     \
                            \"nr1\" : 1,       \
                            \"nr2\" : 2.34     \
                         }";

    jsonman_parse(two_strings);
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


    return 0;
}
