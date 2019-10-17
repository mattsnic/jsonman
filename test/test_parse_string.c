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

    size_t key_length;
    get_key_length(1, &key_length);
    ASSERT_EQUALS(6, key_length);

    char key[key_length + 1];
    key[key_length] = '\0';
    get_key(1, key);
    ASSERT_STRING_EQUALS("\"key1\"", key);

    size_t value_length;
    get_value_length(1, &value_length);
    ASSERT_EQUALS(8, value_length);

    char value[value_length + 1];
    value[value_length] = '\0';
    get_value_as_string(1, value);
    ASSERT_STRING_EQUALS("\"value1\"", value);

    jsonman_free();
    MEM_ALLOC_CHECK;

    /*
     * Testing with two strings
     */
    char* two_strings = "{                            \
                            \"key1\" : \"value1\",    \
                            \"key2\" : \"value2\"     \
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

    id = 2;
    get_key_length(id, &key_length);
    ASSERT_EQUALS(6, key_length);

    char key2[key_length + 1];
    key2[key_length] = '\0';
    get_key(id, key2);
    ASSERT_STRING_EQUALS("\"key2\"", key2);

    get_value_length(id, &value_length);
    ASSERT_EQUALS(8, value_length);

    char value2[value_length + 1];
    value2[value_length] = '\0';
    get_value_as_string(id, value2);
    ASSERT_STRING_EQUALS("\"value2\"", value2);

    jsonman_free();
    MEM_ALLOC_CHECK;

    OK;
    return 0;
}
