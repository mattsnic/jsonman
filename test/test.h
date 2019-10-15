#include "../src/jsonman.h"

#define ERROR do { \
                    FILE* f; \
                    fopen_s(&f, "./test_errors.txt", "a"); \
                    fprintf(f, "Error occured in file %s on line %d\n", __FILE__, __LINE__);  \
                    fflush(f); \
                    fclose(f); \
                 } while(0)

int mem_alloc_ok()
{
    if (MALLOCS <= 0)
    {
        ERROR;
        return 0;
    }
    if (FREES <= 0)
    {
        ERROR;
        return 0;
    }
    if (MALLOCS != FREES)
    {
        ERROR;
        return 0;
    }
    return 1;
}

int error_code()
{
    if (jsonic_get_last_error() == JSONIC_NO_ERROR)
    {
        return 0;
    }
    return 1;
}