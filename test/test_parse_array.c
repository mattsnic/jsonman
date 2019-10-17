#include "test.h"

int main()
{
    /*
     * Testing with one element
     */
    char* one_element = "{                           \
                            [                        \
                                \"value1\"           \
                            ]                        \
                        }";

    jsonman_parse(one_element);
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
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_STRING);
            break;
        case 3:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY_END);
            break;
        case 4:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = next_id(id);
    } while (id > 0);

    ASSERT_VALUE(2, 8, "\"value1\"");

    jsonman_free();
    MEM_ALLOC_CHECK;

    /*
     * Testing with multiple elements
     */
    char* multiple_elements =    "{                                 \
                                    [                               \
                                        \"false\",                  \
                                        12,                         \
                                        15.03,                      \
                                        true,                       \
                                        {                           \
                                            \"key1\" : \"value1\"   \
                                        },                          \
                                        [                           \
                                            false                   \
                                        ]                           \
                                    ]                               \
                                  }";

    jsonman_parse(multiple_elements);
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
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY);
            break;
        case 2:
            ASSERT_EQUALS(get_type(id), JSONMAN_STRING);
            break;
        case 3:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 4:
            ASSERT_EQUALS(get_type(id), JSONMAN_NUMBER);
            break;
        case 5:
            ASSERT_EQUALS(get_type(id), JSONMAN_BOOLEAN);
            break;
        case 6:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT);
            break;
        case 7:
            ASSERT_EQUALS(get_type(id), JSONMAN_STRING);
            break;
        case 8:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        case 9:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY);
            break;
        case 10:
            ASSERT_EQUALS(get_type(id), JSONMAN_BOOLEAN);
            break;
        case 11:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY_END);
            break;
        case 12:
            ASSERT_EQUALS(get_type(id), JSONMAN_ARRAY_END);
            break;
        case 13:
            ASSERT_EQUALS(get_type(id), JSONMAN_OBJECT_END);
            break;
        default:
            ERROR_AND_RETURN;
        }
        id = next_id(id);
    } while (id > 0);

    ASSERT_VALUE(2, 7, "\"false\"");
    ASSERT_VALUE(3, 2, "12");
    ASSERT_VALUE(4, 5, "15.03");
    ASSERT_VALUE(5, 4, "true");
    ASSERT_KEY(7, 6, "\"key1\"");
    ASSERT_VALUE(7, 8, "\"value1\"");
    ASSERT_VALUE(10, 5, "false");

    jsonman_free();
    MEM_ALLOC_CHECK;

    OK;
}

