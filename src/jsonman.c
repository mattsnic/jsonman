/**************************/
/*                        */
/*     Implementation     */
/*                        */
/**************************/

#include "jsonman.h"

static const char _COLON = ':';
static const char _COMMA = ',';
static const char _POINT = '.';
static const char _DOUBLE_QUOTE = '"';
static const char _BACKSLASH = '\\';
static const char _OBJECT_START = '{';
static const char _ARRAY_START = '[';
static const char _OBJECT_END = '}';
static const char _ARRAY_END = ']';
static const char JSONIC_SPACE = ' ';

const char JSONIC_OBJECT = 1;
const char JSONIC_ARRAY = 2;
const char JSONIC_STRING = 3;
const char JSONIC_NUMBER = 4;
const char JSONIC_BOOLEAN = 5;
const char JSONIC_NAMED_OBJECT = 6;
const char JSONIC_NAMED_ARRAY = 7;
const char JSONIC_OBJECT_END = 9;
const char JSONIC_ARRAY_END = 10;
const char JSONIC_UNQUOTED_VALUE = 11;


#ifdef _WIN32
static const char* _NEW_LINE = "\r\n";
#else
static const char* _NEW_LINE = "\n";
#endif // _WIN32

static const short INDENT_SPACES = 4;


uint MALLOCS;
uint FREES;

jsonic_element_t* element_array = NULL;
char* value_array = NULL;


static jsonic_error_t jsonic_last_error = JSONIC_NO_ERROR;
static size_t jsonic_error_pos = 0;

static char* serialized_output = NULL;


static void* jsonic_malloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        jsonic_last_error = JSONIC_ERROR_MEM_ALLOC;
        perror("");
    }
    else {
#ifdef JSONIC_TEST
        ++MALLOCS;
#endif
    }
    return ptr;
}

static void jsonic_free_value(void* ptr)
{
    if (ptr)
    {
        free(ptr);
    }
#ifdef JSONIC_TEST
    ++FREES;
#endif

}

static uint is_boolean(char* json, size_t* start, size_t* end) {

    int length = *end - *start;
    if (length != 4 && length != 5)
    {
        return 0;
    }
    if (length == 4)
    {
        return (tolower(json[*start]) == 't'
            && tolower(json[(*start) + 1]) == 'r'
            && tolower(json[(*start) + 2]) == 'u'
            && tolower(json[(*start) + 3]) == 'e');
    }
    return (tolower(json[*start]) == 'f'
        && tolower(json[(*start) + 1]) == 'a'
        && tolower(json[(*start) + 2]) == 'l'
        && tolower(json[(*start) + 3]) == 's'
        && tolower(json[(*start) + 4]) == 'e');
}

static uint is_number(char* json, size_t* start, size_t* end) {

    int point_count = 0;
    for (size_t i = *start; i < *end; i++)
    {
        if (i == *start && json[i] == '-')
        {
            continue;
        }
        if (isdigit(json[i]))
        {
            continue;
        }
        else if (json[i] == _POINT)
        {
            point_count++;
        }
        else
        {
            return 0;
        }
    }
    return (point_count <= 1 && isdigit(json[(*end) - 1]));
}

uint jsonic_new()
{
    return jsonic_last_error = JSONIC_NO_ERROR;
}


void jsonic_free()
{
    if (value_array)
    {
        jsonic_free_value(value_array);
        value_array = NULL;
    }
    if (element_array)
    {
        jsonic_free_value(element_array);
        element_array = NULL;
    }
    if (serialized_output)
    {
        jsonic_free_value(serialized_output);
        serialized_output = NULL;
    }
}


static void parse(char* json, size_t* nr_objects, size_t* values_size)
{
    if (*nr_objects == 0 && *values_size == 0)
    {
        jsonic_last_error = JSONIC_ERROR_NO_DATA;
        return;
    }
    if (element_array)
    {
        jsonic_free_value(element_array);
    }
    if (value_array)
    {
        jsonic_free_value(value_array);
    }
    element_array = jsonic_malloc((*nr_objects) * sizeof(jsonic_element_t));
    if (!element_array) return;

    value_array = jsonic_malloc((*values_size) + 1);
    if (!value_array) return;
    value_array[*values_size] = '\0';
    char stack[STACK_SIZE];

    int stackpos = -1;

    size_t len = strlen(json);
    size_t pos = 0;
    char expect_new = 1;
    size_t element_count = 0;

    element_array[element_count].key_start = 0;
    element_array[element_count].key_end = 0;
    element_array[element_count].value_start = 0;
    element_array[element_count].value_end = 0;

    size_t i = 0;
    while (i < *nr_objects)
    {
        if (pos > 215)
            printf("");

        if (stackpos == STACK_SIZE)
        {
            jsonic_last_error = JSONIC_ERROR_STACK_OVERFLOW;
            return;
        }
        if (pos == len) break;
        while (isspace(json[pos])) ++pos;
        if (pos == len) break;

        if (json[pos] == _COLON)  //between key and value
        {
            ++pos;
            while (isspace(json[pos])) ++pos;
            expect_new = 0;
        }
        else if (json[pos] == _COMMA)  //end of value
        {
            ++pos;
            while (isspace(json[pos])) ++pos;
            //check that comma is not followed by object_end or array_end
            if (json[pos] == _OBJECT_END || json[pos] == _ARRAY_END)
            {
                jsonic_last_error = JSONIC_ERROR_INVALID_INPUT;  //value not followed by comma when should
                jsonic_error_pos = pos;
                return;
            }
            expect_new = 1;
        }
        else if (pos > 0 && !expect_new)
        {
            //Find latest start element for structure (object or array)
            size_t backwards_count = 0;
            char error = 0;
            int object_level = 1;
            int array_level = 1;
            if (json[pos] != _OBJECT_END && json[pos] != _ARRAY_END)
            {
                error = 1;
            }
            else if (json[pos] == _OBJECT_END)
            {
                object_level = 1;
                array_level = 0;
            }
            else if (json[pos] == _ARRAY_END)
            {
                object_level = 0;
                array_level = 1;
            }
            if (!error)
            {
                char type;
                do
                {
                    size_t temp_pos = element_count - (++backwards_count);
                    type = element_array[element_count - backwards_count].type;
                    switch (type) {
                    case _OBJECT_START:
                        --object_level;
                        break;
                    case _OBJECT_END:
                        ++object_level;
                        break;
                    case _ARRAY_START:
                        --array_level;
                        break;
                    case _ARRAY_END:
                        ++array_level;
                        break;
                    }
                    if (backwards_count == 0) 
                    {
                        break;
                    }
                } while ((object_level != 0 && array_level != 0) && (type != _OBJECT_START && type != _ARRAY_START));

                if (type == _OBJECT_START && json[pos] != _OBJECT_END)
                {
                    error = 1;
                }
                else if (type == _ARRAY_START && json[pos] != _ARRAY_END)
                {
                    error = 1;
                }
            }
            if (error)
            {
                jsonic_last_error = JSONIC_ERROR_INVALID_INPUT;  //value not followed by comma when should
                jsonic_error_pos = pos;
                return;
            }
        }

        switch (json[pos])
        {
        case _OBJECT_START:
        {
            element_array[element_count].type = JSONIC_OBJECT;
            ++element_count;
            element_array[element_count].key_start = 0;
            element_array[element_count].key_end = 0;
            element_array[element_count].value_start = 0;
            element_array[element_count].value_end = 0;
            ++pos;
            expect_new = 1;
            stack[++stackpos] = JSONIC_OBJECT;
            break;
        }
        case _OBJECT_END:
        {
            element_array[element_count].type = JSONIC_OBJECT_END;
            ++element_count;
            if (element_count < (*nr_objects))
            {
                element_array[element_count].key_start = 0;
                element_array[element_count].key_end = 0;
                element_array[element_count].value_start = 0;
                element_array[element_count].value_end = 0;
            }
            ++pos;
            expect_new = 1;
            stack[stackpos--] = 0;
            if (stack[stackpos] == JSONIC_NAMED_OBJECT)
            {
                stack[stackpos--] = 0;
            }
            break;
        }
        case _ARRAY_START:
        {
            element_array[element_count].type = JSONIC_ARRAY;
            ++element_count;
            element_array[element_count].key_start = 0;
            element_array[element_count].key_end = 0;
            element_array[element_count].value_start = 0;
            element_array[element_count].value_end = 0;
            ++pos;
            expect_new = 1;
            stack[++stackpos] = JSONIC_ARRAY;
            break;
        }
        case _ARRAY_END:
        {
            element_array[element_count].type = JSONIC_ARRAY_END;
            ++element_count;
            if (element_count < (*nr_objects))
            {
                element_array[element_count].key_start = 0;
                element_array[element_count].key_end = 0;
                element_array[element_count].value_start = 0;
                element_array[element_count].value_end = 0;
            }
            ++pos;
            expect_new = 1;
            stack[stackpos--] = 0;
            if (stack[stackpos] == JSONIC_NAMED_ARRAY)
            {
                stack[stackpos--] = 0;
            }
            break;
        }
        default:
        {
            size_t temp_pos = pos;
            if (json[temp_pos] == '\"')  //Quoted key or value
            {
                ++temp_pos;
                int found = 0;
                while (!found)
                {
                    if (json[temp_pos] == '\"' && json[temp_pos - 1] != '\\')
                    {
                        found = 1;
                    }
                    ++temp_pos;
                }

                if (stack[stackpos] == JSONIC_OBJECT || stack[stackpos] == JSONIC_ARRAY)
                {
                    size_t next_char_at = temp_pos;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }

                    int increment_element_count = 1;
                    if (json[next_char_at] == _COLON)  //Named object / array or beginning of key/value element
                    {
                        ++next_char_at;
                        while (next_char_at < len && isspace(json[next_char_at])) {
                            ++next_char_at;
                        }
                        if (json[next_char_at] == _OBJECT_START)
                        {
                            element_array[element_count].type = JSONIC_NAMED_OBJECT;
                            stack[++stackpos] = JSONIC_NAMED_OBJECT;
                        }
                        else if (json[next_char_at] == _ARRAY_START)
                        {
                            element_array[element_count].type = JSONIC_NAMED_ARRAY;
                            stack[++stackpos] = JSONIC_NAMED_ARRAY;
                        }
                        else if (json[next_char_at] == _DOUBLE_QUOTE)
                        {
                            element_array[element_count].type = JSONIC_STRING;
                            stack[++stackpos] = JSONIC_STRING;
                            increment_element_count = 0;
                        }
                        else
                        {
                            element_array[element_count].type = JSONIC_UNQUOTED_VALUE;
                            stack[++stackpos] = JSONIC_UNQUOTED_VALUE;
                            increment_element_count = 0;
                        }
                        element_array[element_count].key_start = pos;
                        element_array[element_count].key_end = temp_pos - 1;
                    }
                    else if (stack[stackpos] == JSONIC_ARRAY) //Array value
                    {
                        element_array[element_count].type = JSONIC_STRING;
                        element_array[element_count].value_start = pos;
                        element_array[element_count].value_end = temp_pos - 1;
                    }
                    else //new key / value pair add to stack
                    {
                        element_array[element_count].type = JSONIC_STRING;
                        element_array[element_count].key_start = pos;
                        element_array[element_count].key_end = temp_pos - 1;
                        stack[++stackpos] = JSONIC_STRING;
                        increment_element_count = 0;
                    }
                    if (increment_element_count)
                    {
                        ++element_count;
                        element_array[element_count].key_start = 0;
                        element_array[element_count].key_end = 0;
                        element_array[element_count].value_start = 0;
                        element_array[element_count].value_end = 0;
                    }
                }
                else if (!expect_new) //String value, add value and pop from stack
                {
                    element_array[element_count].type = JSONIC_STRING;
                    element_array[element_count].value_start = pos;
                    element_array[element_count].value_end = temp_pos - 1;
                    stack[stackpos--] = 0;

                    ++element_count;
                    element_array[element_count].key_start = 0;
                    element_array[element_count].key_end = 0;
                    element_array[element_count].value_start = 0;
                    element_array[element_count].value_end = 0;
                }
                else
                {
                    printf("Error\n");
                }
            }
            else  //Unquoted string / value
            {
                ++temp_pos;
                int found = 0;
                while (!found)
                {
                    if (json[temp_pos] == _COLON
                        || json[temp_pos] == _COMMA
                        || json[temp_pos] == _OBJECT_END
                        || json[temp_pos] == _ARRAY_END)
                    {
                        found = 1;
                    }
                    else
                    {
                        ++temp_pos;
                    }
                }
                while (isspace(json[temp_pos - 1]))
                {
                    --temp_pos;
                }

                char type;
                if (is_number(json, &pos, &temp_pos))
                {
                    type = JSONIC_NUMBER;
                }
                else if (is_boolean(json, &pos, &temp_pos))
                {
                    type = JSONIC_BOOLEAN;
                }
                else
                {
                    type = JSONIC_UNQUOTED_VALUE;
                }

                if (stack[stackpos] == JSONIC_OBJECT || stack[stackpos] == JSONIC_ARRAY) //Number or boolean array value, don't add to stack
                {
                    size_t next_char_at = temp_pos;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }

                    if (json[next_char_at] == _COLON)  //Named object / array
                    {
                        ++next_char_at;
                        while (next_char_at < len && isspace(json[next_char_at])) {
                            ++next_char_at;
                        }
                        if (json[next_char_at] == _OBJECT_START)
                        {
                            element_array[element_count].type = JSONIC_NAMED_OBJECT;
                            stack[++stackpos] = JSONIC_NAMED_OBJECT;
                        }
                        else if (json[next_char_at] == _ARRAY_START)
                        {
                            element_array[element_count].type = JSONIC_NAMED_ARRAY;
                            stack[++stackpos] = JSONIC_NAMED_ARRAY;
                        }
                        else
                        {
                            jsonic_last_error = JSONIC_ERROR_INVALID_INPUT;
                            jsonic_error_pos = next_char_at;
                            return;
                        }
                        element_array[element_count].key_start = pos;
                        element_array[element_count].key_end = temp_pos - 1;
                    }
                    else if (stack[stackpos] == JSONIC_ARRAY)//Array value
                    {
                        if (type == JSONIC_UNQUOTED_VALUE)
                        {
                            jsonic_last_error = JSONIC_ERROR_INVALID_INPUT;
                            jsonic_error_pos = next_char_at;
                            return;
                        }
                        element_array[element_count].type = type;
                        element_array[element_count].value_start = pos;
                        element_array[element_count].value_end = temp_pos - 1;
                    }
                    else { //new key / value pair add to stack
                        element_array[element_count].type = type;
                        element_array[element_count].key_start = pos;
                        element_array[element_count].key_end = temp_pos - 1;
                        stack[++stackpos] = JSONIC_UNQUOTED_VALUE;
                    }
                    ++element_count;
                    element_array[element_count].key_start = 0;
                    element_array[element_count].key_end = 0;
                    element_array[element_count].value_start = 0;
                    element_array[element_count].value_end = 0;
                }
                else if (!expect_new) //Number or boolean value, add value and pop from stack
                {
                    element_array[element_count].type = type;
                    element_array[element_count].value_start = pos;
                    element_array[element_count].value_end = temp_pos - 1;
                    stack[stackpos--] = 0;

                    ++element_count;
                    element_array[element_count].key_start = 0;
                    element_array[element_count].key_end = 0;
                    element_array[element_count].value_start = 0;
                    element_array[element_count].value_end = 0;
                }
                else
                {
                    printf("Error2\n");
                }

            }
            size_t length = temp_pos - pos;
            pos += length;
        }
        }
        ++i;
    }
}

static void init_parse(char* json, size_t* nr_objects, size_t* values_size)
{
    size_t len = strlen(json);
    size_t pos = 0;
    char expect_new = 1;

    size_t i = 0;
    while (i < len)
    {
        if (pos == len) break;
        while (isspace(json[pos])) ++pos;
        if (pos == len) break;

        if (json[pos] == _COLON)  //between key and value
        {
            ++pos;
            while (isspace(json[pos])) ++pos;
            expect_new = 0;
        }
        else if (json[pos] == _COMMA)  //end of value
        {
            ++pos;
            while (isspace(json[pos])) ++pos;
            expect_new = 1;
        }

        switch (json[pos])
        {
        case _OBJECT_START:
        case _OBJECT_END:
        case _ARRAY_START:
        case _ARRAY_END:
            ++(*nr_objects);
            ++pos;
            expect_new = 1;
            break;
        default:
            if (expect_new)
            {
                ++(*nr_objects);
            }
            size_t temp_pos = pos;
            if (json[temp_pos] == '\"')
            {
                ++temp_pos;
                int found = 0;
                while (!found)
                {
                    if (json[temp_pos] == '\"' && json[temp_pos - 1] != '\\')
                    {
                        found = 1;
                    }
                    ++temp_pos;
                }
            }
            else
            {
                ++temp_pos;
                int found = 0;
                while (!found)
                {
                    if (json[temp_pos] == _COLON || json[temp_pos] == _COMMA || json[temp_pos] == _OBJECT_END || json[temp_pos] == _ARRAY_END)
                    {
                        found = 1;
                    }
                    else
                    {
                        ++temp_pos;
                    }
                }
                while (isspace(json[temp_pos - 1]))
                {
                    --temp_pos;
                }
            }
            size_t length = temp_pos - pos;
            pos += length;
            (*values_size) += length;
        }
        ++i;
    }
}



void jsonic_parse(char* json)
{

#ifdef JSONIC_TEST
    printf("Test is defined\n");
    MALLOCS = 0;
    FREES = 0;
#endif // JSONIC_TEST

    size_t nr_objects = 0;
    size_t values_size = 0;
    init_parse(json, &nr_objects, &values_size);
    parse(json, &nr_objects, &values_size);

#ifdef JSONIC_TEST

    printf("Nr objects: %d\n", nr_objects);
    printf("Values size: %d\n", values_size);
#endif
}

static void calculate_size(jsonic_element_t* element, uint* output_size, jsonic_print_t* type, uint* level)
{
}

static void serialize(jsonic_element_t* element, char* output, jsonic_print_t* type, uint* level)
{
}

char* jsonic_serialize(jsonic_element_t* root_element, jsonic_print_t print_type, uint* output_size)
{
    jsonic_last_error = JSONIC_NO_ERROR;

    jsonic_element_t* structure = NULL;
    jsonic_print_t type;
    if (serialized_output)
    {
        jsonic_free_value(serialized_output);
    }


    uint size = 0;
    uint level = 0;
    calculate_size(structure, &size, &type, &level);
    serialized_output = jsonic_malloc((size + 1) * sizeof(char));
    serialized_output[size] = '\0';
    level = 0;
    serialize(structure, serialized_output, &type, &level);

    if (output_size)
    {
        *output_size = size;
    }
    return serialized_output;
}


jsonic_error_t jsonic_get_last_error()
{
    return jsonic_last_error;
}

int jsonic_get_error_pos()
{
    return jsonic_error_pos;
}