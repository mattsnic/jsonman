add_executable (Test_parse_string test_parse_string.c)
target_link_libraries( Test_parse_string jsonmanlib )

add_executable (Test_parse_number test_parse_number.c)
target_link_libraries( Test_parse_number jsonmanlib )

add_executable (Test_parse_boolean test_parse_boolean.c)
target_link_libraries( Test_parse_boolean jsonmanlib )

add_executable (Test_parse_array test_parse_array.c)
target_link_libraries( Test_parse_array jsonmanlib )

add_executable (Test_parse_key_without_quotes test_parse_key_without_quotes.c)
target_link_libraries( Test_parse_key_without_quotes jsonmanlib )

add_executable (Test_parse_named_object test_parse_named_object.c)
target_link_libraries( Test_parse_named_object jsonmanlib )

add_executable (Test_parse_named_array test_parse_named_array.c)
target_link_libraries( Test_parse_named_array jsonmanlib )

add_test (NAME Parse_string COMMAND Test_parse_string)
add_test (NAME Parse_number COMMAND Test_parse_number)
add_test (NAME Parse_boolean COMMAND Test_parse_boolean)
add_test (NAME Parse_array COMMAND Test_parse_array)
add_test (NAME Parse_key_without_quotes COMMAND Test_parse_key_without_quotes)
add_test (NAME Parse_named_object COMMAND Test_parse_named_object)
add_test (NAME Parse_named_array COMMAND Test_parse_named_array)

