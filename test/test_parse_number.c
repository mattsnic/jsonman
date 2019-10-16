#include "test.h"

int main()
{
    /*
     * Testing with one number
     */
    char* one_number =    "{                         \
                                \"int_nr\" : 5       \
                           }";

    jsonman_parse(one_number);
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
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
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
     * Testing with two numbers
     */
    char* two_numbers = "{                     \
                            \"nr1\" : 1,       \
                            \"nr2\" : 2.34     \
                         }";

    jsonman_parse(two_numbers);
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
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
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

    return 0;
}
