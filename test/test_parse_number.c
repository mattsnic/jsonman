#include "test.h"

int main()
{
    /*
     * Testing with one number
     */
    char* one_number =    "{                         \
                                \"int_nr\" : 5       \
                           }";

    jm_parse(one_number);
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
            ASSERT_EQUALS(jm_get_type(id), JM_NUMBER);
            break;
        case 2:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = jm_next_id(id);
    } while (id > 0);

    ASSERT_KEY(1, 8, "\"int_nr\"");
    ASSERT_VALUE(1, 1, "5");

    jm_free();
    MEM_ALLOC_CHECK;


    /*
     * Testing with two numbers
     */
    char* two_numbers = "{                     \
                            \"nr1\" : 1,       \
                            \"nr2\" : 2.34     \
                         }";

    jm_parse(two_numbers);
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
            ASSERT_EQUALS(jm_get_type(id), JM_NUMBER);
            break;
        case 2:
            ASSERT_EQUALS(jm_get_type(id), JM_NUMBER);
            break;
        case 3:
            ASSERT_EQUALS(jm_get_type(id), JM_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = jm_next_id(id);
    } while (id > 0);

    ASSERT_KEY(1, 5, "\"nr1\"");
    ASSERT_VALUE(1, 1, "1");

    ASSERT_KEY(2, 5, "\"nr2\"");
    ASSERT_VALUE(2, 4, "2.34");

    jm_free();
    MEM_ALLOC_CHECK;

    OK;
}
