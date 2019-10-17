#include "test.h"

#define ALLOW_UNQUOTED_JSON_KEYS

int main()
{
    /*
     * Testing with one string
     */
    char* one_string = "{                            \
                            key1 : \"value1\"    \
                        }";

    jsonman_parse(one_string);
    ERROR_CODE_CHECK;

    int id = 0;
    do
    {
        switch (id)
        {
        case 0:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT);
            break;
        case 1:
            ASSERT_EQUALS(get_type(id), JSONMAN_STRING);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = next_id(id);
    } while (id > 0);

    jsonman_free();
    MEM_ALLOC_CHECK;

    /*
     * Testing with two strings
     */
    char* two_strings = "{                            \
                            key1 : \"value1\",    \
                            key2 : \"value2\"     \
                         }";

    jsonman_parse(two_strings);
    ERROR_CODE_CHECK;

    id = 0;
    do
    {
        switch (id)
        {
        case 0:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT);
            break;
        case 1:
            ASSERT_EQUALS(get_type(id), JSONMAN_STRING);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_STRING);
            break;
        case 3:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = next_id(id);
    } while (id > 0);


    jsonman_free();
    MEM_ALLOC_CHECK;

    OK;
    return 0;
}

