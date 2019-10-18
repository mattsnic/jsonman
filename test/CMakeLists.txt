add_library (jmlib_TEST STATIC "../src/jsonman.h" "../src/jsonman.c")
target_compile_definitions(jmlib_TEST PRIVATE JM_TEST=1)


add_executable (Test_parse_string test_parse_string.c)
target_link_libraries( Test_parse_string jmlib_TEST )

add_executable (Test_parse_number test_parse_number.c)
target_link_libraries( Test_parse_number jmlib_TEST )

add_executable (Test_parse_boolean test_parse_boolean.c)
target_link_libraries( Test_parse_boolean jmlib_TEST )

add_executable (Test_parse_array test_parse_array.c)
target_link_libraries( Test_parse_array jmlib_TEST )

add_executable (Test_parse_named_object test_parse_named_object.c)
target_link_libraries( Test_parse_named_object jmlib_TEST )

add_executable (Test_parse_named_array test_parse_named_array.c)
target_link_libraries( Test_parse_named_array jmlib_TEST )

add_test (NAME Parse_string COMMAND Test_parse_string)
add_test (NAME Parse_number COMMAND Test_parse_number)
add_test (NAME Parse_boolean COMMAND Test_parse_boolean)
add_test (NAME Parse_array COMMAND Test_parse_array)
add_test (NAME Parse_named_object COMMAND Test_parse_named_object)
add_test (NAME Parse_named_array COMMAND Test_parse_named_array)

# Building version that allows json keys with no quotation marks and do the same tests again plus one more
add_library (jmlib_TEST_ALLOW_NO_QUOTE_KEYS STATIC "../src/jsonman.h" "../src/jsonman.c")
target_compile_definitions(jmlib_TEST_ALLOW_NO_QUOTE_KEYS PRIVATE JM_TEST=1 JM_ALLOW_UNQUOTED_JSON_KEYS=1)

add_executable (Test_parse_string2 test_parse_string.c)
target_link_libraries( Test_parse_string2 jmlib_TEST_ALLOW_NO_QUOTE_KEYS )

add_executable (Test_parse_number2 test_parse_number.c)
target_link_libraries( Test_parse_number2 jmlib_TEST_ALLOW_NO_QUOTE_KEYS )

add_executable (Test_parse_boolean2 test_parse_boolean.c)
target_link_libraries( Test_parse_boolean2 jmlib_TEST_ALLOW_NO_QUOTE_KEYS )

add_executable (Test_parse_array2 test_parse_array.c)
target_link_libraries( Test_parse_array2 jmlib_TEST_ALLOW_NO_QUOTE_KEYS )

add_executable (Test_parse_key_without_quotes test_parse_key_without_quotes.c)
target_link_libraries( Test_parse_key_without_quotes jmlib_TEST_ALLOW_NO_QUOTE_KEYS )

add_executable (Test_parse_named_object2 test_parse_named_object.c)
target_link_libraries( Test_parse_named_object2 jmlib_TEST_ALLOW_NO_QUOTE_KEYS )

add_executable (Test_parse_named_array2 test_parse_named_array.c)
target_link_libraries( Test_parse_named_array2 jmlib_TEST_ALLOW_NO_QUOTE_KEYS )

add_test (NAME Parse_string2 COMMAND Test_parse_string2)
add_test (NAME Parse_number2 COMMAND Test_parse_number2)
add_test (NAME Parse_boolean2 COMMAND Test_parse_boolean2)
add_test (NAME Parse_array2 COMMAND Test_parse_array2)
add_test (NAME Parse_key_without_quotes COMMAND Test_parse_key_without_quotes)
add_test (NAME Parse_named_object2 COMMAND Test_parse_named_object2)
add_test (NAME Parse_named_array2 COMMAND Test_parse_named_array2)
