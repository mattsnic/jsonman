#add_executable (jsonman "jsonman.h" "jsonman.c" "run.c")

add_library (jmlib STATIC "jsonman.h" "jsonman.c")

target_compile_definitions(jmlib PUBLIC)
