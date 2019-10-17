#include "test.h"

int main()
{
    /*
     * Testing with one named object
     */
    char* one_object = "{                       \
                            \"key1\" : {        \
                                \"key2\" : 10   \
                            }                   \
                        }";

    jsonman_parse(one_object);
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
            ASSERT_EQUALS(get_type(id), JSONMAN_NAMED_OBJECT);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT);
            break;
        case 3:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 4:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        case 5:
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
     * Testing with two named objects
     */

    char* two_objects = "{                      \
                            \"key1\" : {        \
                                \"key2\" : 10   \
                            }                   \
                            \"key3\" : {        \
                                \"key4\" : 20   \
                            }                   \
                        }";

    jsonman_parse(two_objects);
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
            ASSERT_EQUALS(get_type(id), JSONMAN_NAMED_OBJECT);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT);
            break;
        case 3:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 4:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        case 5:
            ASSERT_EQUALS(get_type(id), JSONMAN_NAMED_OBJECT);
            break;
        case 6:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT);
            break;
        case 7:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 8:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        case 9:
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
}

