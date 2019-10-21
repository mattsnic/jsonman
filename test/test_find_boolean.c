#include "test.h"

int main()
{
    /*
     * Testing find functions
     */
    char* json = "\
[                                                                                  \
  {                                                                                \
    \"_id\": \"5daa0a31c4bc796ee6754ef7\",                                         \
    \"name\": \"Kelly Erickson\",                                                  \
    \"gender\": \"female\",                                                        \
    \"company\": \"FLEXIGEN\",                                                     \
    \"email\": \"kellyerickson@flexigen.com\",                                     \
    \"phone\": \"+1 (960) 518-2947\",                                              \
    \"address\": \"645 Herkimer Court, Escondida, Puerto Rico, 535\",              \
    \"about\": \"Adipisicing anim consequat deserunt nulla irure nulla ut.\r\n\",  \
    \"registered\": \"2019-10-18T08:16:52 -02:00\",                                \
    \"latitude\": 45.603709,                                                       \
    \"longitude\": -83.998699,                                                     \
    \"married\": true,                                                             \
    {                                                                              \
		\"shoe_size\": 45                                                          \
    },                                                                             \
    \"tags\": [                                                                    \
      \"laborum\",                                                                 \
      \"enim\",                                                                    \
      \"et\",                                                                      \
      \"deserunt\",                                                                \
      \"laboris\",                                                                 \
      \"tempor\",                                                                  \
      \"anim\",                                                                    \
	  \"tags2\": [                                                                 \
		1,                                                                         \
		2,                                                                         \
		3                                                                          \
	  ]                                                                            \
    ],                                                                             \
    \"friends\": [                                                                 \
      {                                                                            \
        \"id\": 0,                                                                 \
        \"name\": \"Bishop Haley\"                                                 \
      },                                                                           \
      {                                                                            \
        \"id\": 1,                                                                 \
        \"name\": \"Curry Kennedy\"                                                \
      },                                                                           \
      {                                                                            \
        \"id\": 2,                                                                 \
        \"name\": \"Mabel Chapman\"                                                \
      }                                                                            \
    ],                                                                             \
    \"drivers_license\": false,                                                    \
    \"greeting\": \"Hello, Kelly Erickson! You have 10 unread messages.\",         \
    \"favoriteFruit\": \"strawberry\"                                              \
  }                                                                                \
]";

    jm_parse(json);
    ERROR_CODE_CHECK;

    int result;
    size_t id_found;
    result = jm_find_next_boolean(0, &id_found, -1, NULL, NULL);
    ASSERT_EQUALS(0, result);
    ASSERT_EQUALS(13, id_found);

    ++id_found;
    result = jm_find_next_boolean(id_found, &id_found, -1, NULL, NULL);
    ASSERT_EQUALS(0, result);
    ASSERT_EQUALS(48, id_found);

    //Find by name (key)
    result = jm_find_next_boolean(0, &id_found, -1, "\"married\"", NULL);
    ASSERT_EQUALS(0, result);
    ASSERT_EQUALS(13, id_found);

    result = jm_find_next_boolean(0, &id_found, -1, "\"drivers_license\"", NULL);
    ASSERT_EQUALS(0, result);
    ASSERT_EQUALS(48, id_found);

    //Find by value
    int value = 1; //true
    result = jm_find_next_boolean(0, &id_found, -1, NULL, &value);
    ASSERT_EQUALS(0, result);
    ASSERT_EQUALS(13, id_found);

    value = 0; //false
    result = jm_find_next_boolean(0, &id_found, -1, NULL, &value);
    ASSERT_EQUALS(0, result);
    ASSERT_EQUALS(48, id_found);

    //Find by level
    result = jm_find_next_boolean(0, &id_found, 1, NULL, NULL);
    ASSERT_EQUALS(0, result);
    ASSERT_EQUALS(13, id_found);
    ASSERT_VALUE(id_found, 4, "true");

    result = jm_find_next_boolean(0, &id_found, 2, NULL, NULL);
    ASSERT_EQUALS(-1, result);
    ASSERT_EQUALS(JM_ERROR_ELEMENT_NOT_FOUND, jm_get_last_error());

    jm_free();
    MEM_ALLOC_CHECK;

    OK;
}
