#include "test.h"

int main()
{
    /*
     * Testing with one named array
     */
    char* one_array = "{                       \
                            \"key1\" : [       \
                                10             \
                            ]                  \
                       }";

    jsonman_parse(one_array);
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
            ASSERT_EQUALS(get_type(id), JSONMAN_NAMED_ARRAY);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY);
            break;
        case 3:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 4:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY_END);
            break;
        case 5:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = next_id(id);
    } while (id > 0);

    ASSERT_KEY(1, 6, "\"key1\"");
    ASSERT_VALUE(3, 2, "10");

    jsonman_free();
    MEM_ALLOC_CHECK;

    /*
     * Testing with two named objects
     */

    char* two_arrays = "{                       \
                            \"key1\" : [        \
                                10              \
                            ]                   \
                            \"key2\" : [        \
                                20              \
                            ]                   \
                        }";

    jsonman_parse(two_arrays);
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
            ASSERT_EQUALS(get_type(id), JSONMAN_NAMED_ARRAY);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY);
            break;
        case 3:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 4:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY_END);
            break;
        case 5:
            ASSERT_EQUALS(get_type(id), JSONMAN_NAMED_ARRAY);
            break;
        case 6:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY);
            break;
        case 7:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 8:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY_END);
            break;
        case 9:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = next_id(id);
    } while (id > 0);

    ASSERT_KEY(1, 6, "\"key1\"");
    ASSERT_VALUE(3, 2, "10");

    ASSERT_KEY(5, 6, "\"key2\"");
    ASSERT_VALUE(7, 2, "20");

    jsonman_free();
    MEM_ALLOC_CHECK;

    OK;
}

