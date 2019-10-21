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
    \"greeting\": \"Hello, Kelly Erickson! You have 10 unread messages.\",         \
    \"favoriteFruit\": \"strawberry\"                                              \
  }                                                                                \
]";

    jm_parse(json);
    ERROR_CODE_CHECK;

    size_t id_found;
    int result;
    result = jm_find_next_array(0, &id_found, -1);
    ASSERT_EQUALS(result, JM_NO_ERROR);
    ASSERT_EQUALS(0, id_found);

    ++id_found;
    result = jm_find_next_array(id_found, &id_found, -1);
    ASSERT_EQUALS(result, JM_NO_ERROR);
    ASSERT_EQUALS(17, id_found);

    ++id_found;
    result = jm_find_next_array(id_found, &id_found, -1);
    ASSERT_EQUALS(result, JM_NO_ERROR);
    ASSERT_EQUALS(26, id_found);

    ++id_found;
    result = jm_find_next_array(id_found, &id_found, -1);
    ASSERT_EQUALS(result, JM_NO_ERROR);
    ASSERT_EQUALS(33, id_found);

    jm_free();
    MEM_ALLOC_CHECK;

    OK;
}
