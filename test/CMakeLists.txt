set(CTEST_OUTPUT_ON_FAILURE TRUE)

add_executable (Test_parse1 test_parse1.c)
target_link_libraries( Test_parse1 jsonmanlib )



add_test (NAME Parse1 COMMAND Test_parse1)