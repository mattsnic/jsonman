#add_executable (jsonman "jsonman.h" "jsonman.c" "run.c")

add_library (jsonmanlib STATIC "jsonman.h" "jsonman.c")

target_compile_definitions(jsonmanlib PRIVATE JSONMAN_TEST=1 ALLOW_UNQUOTED_JSON_KEYS=1)
