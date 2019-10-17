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

    jm_parse(one_object);
    ERROR_CODE_CHECK;

    int id = 0;
    do
    {
        switch (id)
        {
        case 0:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT);
            break;
        case 1:
            ASSERT_EQUALS(jm_get_type(id), JM_NAMED_OBJECT);
            break;
        case 2:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT);
            break;
        case 3:
            ASSERT_EQUALS(jm_get_type(id), JM_NUMBER);
            break;
        case 4:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        case 5:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = jm_next_id(id);
    } while (id > 0);

    ASSERT_KEY(1, 6, "\"key1\"");
    ASSERT_KEY(3, 6, "\"key2\"");
    ASSERT_VALUE(3, 2, "10");

    jm_free();
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

    jm_parse(two_objects);
    ERROR_CODE_CHECK;

    id = 0;
    do
    {
        switch (id)
        {
        case 0:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT);
            break;
        case 1:
            ASSERT_EQUALS(jm_get_type(id), JM_NAMED_OBJECT);
            break;
        case 2:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT);
            break;
        case 3:
            ASSERT_EQUALS(jm_get_type(id), JM_NUMBER);
            break;
        case 4:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        case 5:
            ASSERT_EQUALS(jm_get_type(id), JM_NAMED_OBJECT);
            break;
        case 6:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT);
            break;
        case 7:
            ASSERT_EQUALS(jm_get_type(id), JM_NUMBER);
            break;
        case 8:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        case 9:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = jm_next_id(id);
    } while (id > 0);

    ASSERT_KEY(1, 6, "\"key1\"");
    ASSERT_KEY(3, 6, "\"key2\"");
    ASSERT_VALUE(3, 2, "10");

    ASSERT_KEY(5, 6, "\"key3\"");
    ASSERT_KEY(7, 6, "\"key4\"");
    ASSERT_VALUE(7, 2, "20");

    jm_free();
    MEM_ALLOC_CHECK;

    OK;
}

