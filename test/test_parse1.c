#include "test.h"

int main()
{
    char* single_string = "{                              \
                                \"key\" : \"value\"         \
                             }";

    jsonic_parse(single_string);
    jsonic_free();

    int ret = 0;
    if (!mem_alloc_ok())
    {
        ERROR;
        ret = 1;
    }

    if (error_code())
    {
        ERROR;
        ret = 1;
    }
    return ret;
}