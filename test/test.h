#include "../src/jsonman.h"

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
                            jsonman_free();                                                           \
                            return 1;                                                                 \
                         } while(0)                                                                   \

#define MEM_ALLOC_CHECK if (MALLOCS <= 0 || FREES <= 0 || MALLOCS != FREES)  ERROR_AND_RETURN

#define ERROR_CODE_CHECK if (jsonman_get_last_error() != JSONMAN_NO_ERROR) ERROR_AND_RETURN

#define ASSERT_EQUALS(expected, actual) if (expected != actual) ERROR_AND_RETURN

#define ASSERT_STRING_EQUALS(expected, actual) if (strcmp(expected, actual) != 0) ERROR_AND_RETURN



