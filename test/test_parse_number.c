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

    id = 1;
    size_t key_length;
    get_key_length(id, &key_length);
    ASSERT_EQUALS(8, key_length);

    char key[key_length + 1];
    key[key_length] = '\0';
    get_key(id, key);
    ASSERT_STRING_EQUALS("\"int_nr\"", key);

    size_t value_length;
    get_value_length(id, &value_length);
    ASSERT_EQUALS(1, value_length);

    char value[value_length + 1];
    value[value_length] = '\0';
    get_value_as_string(id, value);
    LOG(value);
    ASSERT_STRING_EQUALS("5", value);


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

    id = 1;
    size_t key_length2;
    get_key_length(id, &key_length2);
    ASSERT_EQUALS(5, key_length2);

    char key2[key_length2 + 1];
    key2[key_length2] = '\0';
    get_key(id, key2);
    ASSERT_STRING_EQUALS("\"nr1\"", key2);

    size_t value_length2;
    get_value_length(id, &value_length2);
    ASSERT_EQUALS(1, value_length2);

    char value2[value_length2 + 1];
    value2[value_length2] = '\0';
    get_value_as_string(id, value2);
    ASSERT_STRING_EQUALS("1", value2);


    id = 2;
    size_t key_length3;
    get_key_length(id, &key_length3);
    ASSERT_EQUALS(5, key_length3);

    char key3[key_length3 + 1];
    key3[key_length3] = '\0';
    get_key(id, key3);
    ASSERT_STRING_EQUALS("\"nr2\"", key3);

    size_t value_length3;
    get_value_length(id, &value_length3);
    ASSERT_EQUALS(4, value_length3);

    char value3[value_length3 + 1];
    value3[value_length3] = '\0';
    get_value_as_string(id, value3);
    ASSERT_STRING_EQUALS("2.34", value3);


    jsonman_free();
    MEM_ALLOC_CHECK;

    OK;
}
