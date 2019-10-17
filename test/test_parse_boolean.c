#include "test.h"

int main()
{
    /*
     * Testing with one string
     */
    char* one_boolean = "{                     \
                            \"key1\" : true    \
                        }";

    jm_parse(one_boolean);
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
            ASSERT_EQUALS(jm_get_type(id), JM_BOOLEAN);
            break;
        case 2:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = jm_next_id(id);
    } while (id > 0);

    ASSERT_KEY(1, 6, "\"key1\"");
    ASSERT_VALUE(1, 4, "true");

    jm_free();
    MEM_ALLOC_CHECK;


    /*
     * Testing with two strings
     */
    char* two_booleans = "{                     \
                            \"key1\" : false,   \
                            \"key2\" : true     \
                          }";

    jm_parse(two_booleans);
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
            ASSERT_EQUALS(jm_get_type(id), JM_BOOLEAN);
            break;
        case 2:
            ASSERT_EQUALS(jm_get_type(id), JM_BOOLEAN);
            break;
        case 3:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = jm_next_id(id);
    } while (id > 0);

    ASSERT_KEY(1, 6, "\"key1\"");
    ASSERT_VALUE(1, 5, "false");

    ASSERT_KEY(2, 6, "\"key2\"");
    ASSERT_VALUE(2, 4, "true");


    jm_free();
    MEM_ALLOC_CHECK;

    OK;
}

