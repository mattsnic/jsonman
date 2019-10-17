#include "../src/jsonman.h"

#define LOG(m) do                                                                         \
              {                                                                           \
                FILE* f;                                                                  \
                fopen_s(&f, "./test_logs.txt", "a");                                      \
                fprintf(f, "%s\n", m);                                                    \
                fflush(f);                                                                \
                fclose(f);                                                                \
              } while(0)       

#define ERROR do                                                                          \
              {                                                                           \
                FILE* f;                                                                  \
                fopen_s(&f, "./test_errors.txt", "a");                                    \
                fprintf(f, "Error occured in file %s on line %d\n", __FILE__, __LINE__);  \
                fflush(f);                                                                \
                fclose(f);                                                                \
              } while(0)                                                                  \

#define ERROR_AND_RETURN do                                                                           \
                         {                                                                            \
                            FILE* f;                                                                  \
                            fopen_s(&f, "./test_errors.txt", "a");                                    \
                            fprintf(f, "Error occured in file %s on line %d\n", __FILE__, __LINE__);  \
                            fflush(f);                                                                \
                            fclose(f);                                                                \
                            jm_free();                                                                \
                            return 1;                                                                 \
                         } while(0)                                                                   \

#define MEM_ALLOC_CHECK if (MALLOCS <= 0 || FREES <= 0 || MALLOCS != FREES)  ERROR_AND_RETURN

#define ERROR_CODE_CHECK if (jm_get_last_error() != JM_NO_ERROR) ERROR_AND_RETURN

#define ASSERT_EQUALS(expected, actual) if (expected != actual) ERROR_AND_RETURN

#define ASSERT_STRING_EQUALS(expected, actual) if (strcmp(expected, actual) != 0) ERROR_AND_RETURN

#define ASSERT_KEY(id, expected_length, expected_value) do {                                             \
                                                            size_t key_length;                           \
                                                            jm_get_key_length(id, &key_length);          \
                                                            ASSERT_EQUALS(expected_length, key_length);  \
                                                                                                         \
                                                            char key[key_length + 1];                    \
                                                            key[key_length] = '\0';                      \
                                                            jm_get_key(id, key);                         \
                                                            ASSERT_STRING_EQUALS(expected_value, key);   \
                                                        } while (0)


#define ASSERT_VALUE(id, expected_length, expected_value) do {                                                 \
                                                                size_t value_length;                           \
                                                                jm_get_value_length(id, &value_length);        \
                                                                ASSERT_EQUALS(expected_length, value_length);  \
                                                                                                               \
                                                                char value[value_length + 1];                  \
                                                                value[value_length] = '\0';                    \
                                                                jm_get_value_as_string(id, value);             \
                                                                ASSERT_STRING_EQUALS(expected_value, value);   \
                                                            } while (0)

#define OK printf("OK\n"); return 0


