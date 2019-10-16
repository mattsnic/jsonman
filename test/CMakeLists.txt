#set(CTEST_OUTPUT_ON_FAILURE TRUE)

add_executable (Test_parse_string test_parse_string.c)
target_link_libraries( Test_parse_string jsonmanlib )

add_executable (Test_parse_number test_parse_number.c)
target_link_libraries( Test_parse_number jsonmanlib )

add_executable (Test_parse_boolean test_parse_boolean.c)
target_link_libraries( Test_parse_boolean jsonmanlib )

add_test (NAME Parse_boolean COMMAND Test_parse_boolean)
add_test (NAME Parse_number COMMAND Test_parse_number)
add_test (NAME Parse_string COMMAND Test_parse_string)

