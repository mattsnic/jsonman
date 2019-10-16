/**************************/
/*                        */
/*     Implementation     */
/*                        */
/**************************/

#include "jsonman.h"

#define RESET_NEXT  do { \
                        element_array[element_count].key_start = 0;   \
                        element_array[element_count].key_end = 0;     \
                        element_array[element_count].value_start = 0; \
                        element_array[element_count].value_end = 0;   \
                    } while (0)

#define SET_KEY_DATA(pos, temp_pos) do {                                                                        \
                                        size_t length = temp_pos - pos;                                         \
                                        element_array[element_count].key_start = value_array_pos;               \
                                        element_array[element_count].key_end = (value_array_pos + length) - 1;  \
                                        for (int i = 0; i < length; i++)                                        \
                                        {                                                                       \
                                            value_array[value_array_pos++] = json[pos + i];                     \
                                        }                                                                       \
                                    } while(0)

#define SET_VALUE_DATA(pos, temp_pos) do {                                                                      \
                                        size_t length = temp_pos - pos;                                         \
                                        element_array[element_count].value_start = value_array_pos;             \
                                        element_array[element_count].value_end = (value_array_pos + length) - 1;\
                                        for (int i = 0; i < length; i++)                                        \
                                        {                                                                       \
                                            value_array[value_array_pos++] = json[pos + i];                     \
                                        }                                                                       \
                                      } while (0)

static const char _COLON = ':';
static const char _COMMA = ',';
static const char _POINT = '.';
static const char _DOUBLE_QUOTE = '"';
static const char _BACKSLASH = '\\';
static const char _OBJECT_START = '{';
static const char _ARRAY_START = '[';
static const char _OBJECT_END = '}';
static const char _ARRAY_END = ']';
static const char JSONMAN_SPACE = ' ';

const char JSONMAN_OBJECT = 1;
const char JSONMAN_ARRAY = 2;
const char JSONMAN_STRING = 3;
const char JSONMAN_NUMBER = 4;
const char JSONMAN_BOOLEAN = 5;
const char JSONMAN_NAMED_OBJECT = 6;
const char JSONMAN_NAMED_ARRAY = 7;
const char JSONMAN_OBJECT_END = 9;
const char JSONMAN_ARRAY_END = 10;
const char JSONMAN_UNQUOTED_VALUE = 11;


#ifdef _WIN32
static const char* _NEW_LINE = "\r\n";
#else
static const char* _NEW_LINE = "\n";
#endif // _WIN32

static const short INDENT_SPACES = 4;


uint MALLOCS;
uint FREES;

jsonman_element_t* element_array = NULL;
char* value_array = NULL;
size_t nr_objects_store = 0;


static jsonman_error_t jsonman_last_error = JSONMAN_NO_ERROR;
static size_t jsonman_error_pos = 0;

static char* serialized_output = NULL;


static void* jsonman_malloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        jsonman_last_error = JSONMAN_ERROR_MEM_ALLOC;
        perror("");
    }
    else {
#ifdef JSONMAN_TEST
        ++MALLOCS;
#endif
    }
    return ptr;
}

static void jsonman_free_value(void* ptr)
{
    if (ptr)
    {
        free(ptr);
    }
#ifdef JSONMAN_TEST
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

uint jsonman_new()
{
    return jsonman_last_error = JSONMAN_NO_ERROR;
}


void jsonman_free()
{
    if (value_array)
    {
        jsonman_free_value(value_array);
        value_array = NULL;
    }
    if (element_array)
    {
        jsonman_free_value(element_array);
        element_array = NULL;
    }
    if (serialized_output)
    {
        jsonman_free_value(serialized_output);
        serialized_output = NULL;
    }
}


static void parse(char* json, size_t* nr_objects, size_t* values_size)
{
    if (*nr_objects == 0 && *values_size == 0)
    {
        jsonman_last_error = JSONMAN_ERROR_NO_DATA;
        return;
    }
    jsonman_free();
    element_array = jsonman_malloc((*nr_objects) * sizeof(jsonman_element_t));
    if (!element_array) return;

    value_array = jsonman_malloc((*values_size) + 1);
    if (!value_array) return;
    value_array[*values_size] = '\0';
    char stack[STACK_SIZE];

    int stackpos = -1;

    size_t len = strlen(json);
    size_t pos = 0;
    size_t value_array_pos = 0;
    char expect_new = 1;
    size_t element_count = 0;

    RESET_NEXT;

    size_t i = 0;
    while (i < *nr_objects)
    {
        if (stackpos == STACK_SIZE)
        {
            jsonman_last_error = JSONMAN_ERROR_STACK_OVERFLOW;
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
                jsonman_last_error = JSONMAN_ERROR_INVALID_INPUT;  //value not followed by comma when should
                jsonman_error_pos = pos;
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
                jsonman_last_error = JSONMAN_ERROR_INVALID_INPUT; 
                jsonman_error_pos = pos;
                return;
            }
        }

        switch (json[pos])
        {
        case _OBJECT_START:
        {
            element_array[element_count].type = JSONMAN_OBJECT;
            ++element_count;
            RESET_NEXT;
            ++pos;
            expect_new = 1;
            stack[++stackpos] = JSONMAN_OBJECT;
            break;
        }
        case _OBJECT_END:
        {
            element_array[element_count].type = JSONMAN_OBJECT_END;
            ++element_count;
            if (element_count < (*nr_objects))
            {
                RESET_NEXT;
            }
            ++pos;
            expect_new = 1;
            stack[stackpos--] = 0;
            if (stack[stackpos] == JSONMAN_NAMED_OBJECT)
            {
                stack[stackpos--] = 0;
            }
            break;
        }
        case _ARRAY_START:
        {
            element_array[element_count].type = JSONMAN_ARRAY;
            ++element_count;
            RESET_NEXT;
            ++pos;
            expect_new = 1;
            stack[++stackpos] = JSONMAN_ARRAY;
            break;
        }
        case _ARRAY_END:
        {
            element_array[element_count].type = JSONMAN_ARRAY_END;
            ++element_count;
            if (element_count < (*nr_objects))
            {
                RESET_NEXT;
            }
            ++pos;
            expect_new = 1;
            stack[stackpos--] = 0;
            if (stack[stackpos] == JSONMAN_NAMED_ARRAY)
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

                if (stack[stackpos] == JSONMAN_OBJECT || stack[stackpos] == JSONMAN_ARRAY)
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
                            element_array[element_count].type = JSONMAN_NAMED_OBJECT;
                            stack[++stackpos] = JSONMAN_NAMED_OBJECT;
                        }
                        else if (json[next_char_at] == _ARRAY_START)
                        {
                            element_array[element_count].type = JSONMAN_NAMED_ARRAY;
                            stack[++stackpos] = JSONMAN_NAMED_ARRAY;
                        }
                        else if (json[next_char_at] == _DOUBLE_QUOTE)
                        {
                            element_array[element_count].type = JSONMAN_STRING;
                            stack[++stackpos] = JSONMAN_STRING;
                            increment_element_count = 0;
                        }
                        else
                        {
                            element_array[element_count].type = JSONMAN_UNQUOTED_VALUE;
                            stack[++stackpos] = JSONMAN_UNQUOTED_VALUE;
                            increment_element_count = 0;
                        }

                        SET_KEY_DATA(pos, temp_pos);
                        /*
                        size_t length = element_array[element_count].key_end - element_array[element_count].key_start;
                        for (int i = 0; i < length; i++)
                        {
                            value_array[value_array_pos++] = json[pos + i];
                        }
                        element_array[element_count].key_start = pos;
                        element_array[element_count].key_end = temp_pos - 1;
                        */
                    }
                    else if (stack[stackpos] == JSONMAN_ARRAY) //Array value
                    {
                        element_array[element_count].type = JSONMAN_STRING;
                        SET_VALUE_DATA(pos, temp_pos);
                        //element_array[element_count].value_start = pos;
                        //element_array[element_count].value_end = temp_pos - 1;
                    }
                    else //new key / value pair add to stack
                    {
                        element_array[element_count].type = JSONMAN_STRING;
                        SET_KEY_DATA(pos, temp_pos);
                        //element_array[element_count].key_start = pos;
                        //element_array[element_count].key_end = temp_pos - 1;
                        stack[++stackpos] = JSONMAN_STRING;
                        increment_element_count = 0;
                    }
                    if (increment_element_count)
                    {
                        ++element_count;
                        RESET_NEXT;
                    }
                }
                else if (!expect_new) //String value, add value and pop from stack
                {
                    element_array[element_count].type = JSONMAN_STRING;
                    SET_VALUE_DATA(pos, temp_pos);
                    //element_array[element_count].value_start = pos;
                    //element_array[element_count].value_end = temp_pos - 1;
                    stack[stackpos--] = 0;

                    ++element_count;
                    RESET_NEXT;

                    size_t next_char_at = temp_pos;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }
                    if (json[next_char_at] != _COMMA && json[next_char_at] != _OBJECT_END && json[next_char_at] != _ARRAY_END)
                    {
                        jsonman_last_error = JSONMAN_ERROR_INVALID_INPUT;
                        jsonman_error_pos = next_char_at;
                        return;
                    }
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
                    type = JSONMAN_NUMBER;
                }
                else if (is_boolean(json, &pos, &temp_pos))
                {
                    type = JSONMAN_BOOLEAN;
                }
                else
                {
                    type = JSONMAN_UNQUOTED_VALUE;
                }

                if (stack[stackpos] == JSONMAN_OBJECT || stack[stackpos] == JSONMAN_ARRAY) //Number or boolean array value, don't add to stack
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
                            element_array[element_count].type = JSONMAN_NAMED_OBJECT;
                            stack[++stackpos] = JSONMAN_NAMED_OBJECT;
                        }
                        else if (json[next_char_at] == _ARRAY_START)
                        {
                            element_array[element_count].type = JSONMAN_NAMED_ARRAY;
                            stack[++stackpos] = JSONMAN_NAMED_ARRAY;
                        }
                        else
                        {
                            jsonman_last_error = JSONMAN_ERROR_INVALID_INPUT;
                            jsonman_error_pos = next_char_at;
                            return;
                        }
                        SET_KEY_DATA(pos, temp_pos);
                        //element_array[element_count].key_start = pos;
                        //element_array[element_count].key_end = temp_pos - 1;
                    }
                    else if (stack[stackpos] == JSONMAN_ARRAY)//Array value
                    {
                        if (type == JSONMAN_UNQUOTED_VALUE)
                        {
                            jsonman_last_error = JSONMAN_ERROR_INVALID_INPUT;
                            jsonman_error_pos = next_char_at;
                            return;
                        }
                        element_array[element_count].type = type;
                        SET_VALUE_DATA(pos, temp_pos);
                        //element_array[element_count].value_start = pos;
                        //element_array[element_count].value_end = temp_pos - 1;
                    }
                    else { //new key / value pair add to stack
                        element_array[element_count].type = type;
                        SET_KEY_DATA(pos, temp_pos);
                        //element_array[element_count].key_start = pos;
                        //element_array[element_count].key_end = temp_pos - 1;
                        stack[++stackpos] = JSONMAN_UNQUOTED_VALUE;
                    }
                    ++element_count;
                    RESET_NEXT;
                }
                else if (!expect_new) //Number or boolean value, add value and pop from stack
                {
                    element_array[element_count].type = type;
                    SET_VALUE_DATA(pos, temp_pos);
                    element_array[element_count].value_start = pos;
                    element_array[element_count].value_end = temp_pos - 1;
                    stack[stackpos--] = 0;

                    ++element_count;
                    RESET_NEXT;

                    size_t next_char_at = temp_pos;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }
                    if (json[next_char_at] != _COMMA && json[next_char_at] != _OBJECT_END && json[next_char_at] != _ARRAY_END)
                    {
                        jsonman_last_error = JSONMAN_ERROR_INVALID_INPUT;
                        jsonman_error_pos = next_char_at;
                        return;
                    }
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
    nr_objects_store = *nr_objects;
}

//************************* Public functions below *************************************


int jsonman_parse(char* json)
{

#ifdef JSONMAN_TEST
    printf("Test is defined\n");
    MALLOCS = 0;
    FREES = 0;
#endif // JSONMAN_TEST

    size_t nr_objects = 0;
    size_t values_size = 0;
    init_parse(json, &nr_objects, &values_size);
    parse(json, &nr_objects, &values_size);

    if (jsonman_last_error != JSONMAN_NO_ERROR)
    {
        return -1;
    }
    return 0;
}

int get_key_length(int id, size_t* out_value)
{
    if (!out_value)
    {
        jsonman_last_error = JSONMAN_ERROR_OUT_PARAMETER_IS_NULL;
        return -1;
    }
    if (nr_objects_store <= id)
    {
        jsonman_last_error = JSONMAN_ERROR_INVALID_ID;
        return -1;
    }
    *out_value = (1 + element_array[id].key_end) - element_array[id].key_start;
    return 0;
}

int get_value_length(int id, size_t* out_value)
{
    if (!out_value)
    {
        jsonman_last_error = JSONMAN_ERROR_OUT_PARAMETER_IS_NULL;
        return -1;
    }
    if (nr_objects_store <= id)
    {
        jsonman_last_error = JSONMAN_ERROR_INVALID_ID;
        return -1;
    }
    if (element_array[id].type == JSONMAN_OBJECT || element_array[id].type == JSONMAN_NAMED_OBJECT
        || element_array[id].type == JSONMAN_ARRAY || element_array[id].type == JSONMAN_NAMED_ARRAY)
    {
        jsonman_last_error = JSONMAN_ERROR_SIMPLE_VALUE_NOT_PRESENT;
        return -1;
    }
    *out_value = (1 + element_array[id].value_end) - element_array[id].value_start;
    return 0;
}

int get_key(int id, char* out_buffer)
{
    if (!out_buffer)
    {
        jsonman_last_error = JSONMAN_ERROR_OUT_PARAMETER_IS_NULL;
        return -1;
    }
    if (nr_objects_store <= id)
    {
        jsonman_last_error = JSONMAN_ERROR_INVALID_ID;
        return -1;
    }
    size_t size = (1 + element_array[id].key_end) - element_array[id].key_start;
    if (size > 0)
    {
        int count = 0;
        for (int i = element_array[id].key_start; i <= element_array[id].key_end; i++)
        {
            out_buffer[count++] = value_array[i];
        }
    }
    return 0;
}

int get_value_as_string(int id, char* out_buffer)
{
    if (!out_buffer)
    {
        jsonman_last_error = JSONMAN_ERROR_OUT_PARAMETER_IS_NULL;
        return -1;
    }
    if (nr_objects_store <= id)
    {
        jsonman_last_error = JSONMAN_ERROR_INVALID_ID;
        return -1;
    }
    if (element_array[id].type == JSONMAN_OBJECT || element_array[id].type == JSONMAN_NAMED_OBJECT
        || element_array[id].type == JSONMAN_ARRAY || element_array[id].type == JSONMAN_NAMED_ARRAY)
    {
        jsonman_last_error = JSONMAN_ERROR_SIMPLE_VALUE_NOT_PRESENT;
        return -1;
    }

    size_t size = element_array[id].value_end - element_array[id].value_start;
    if (size > 0)
    {
        int count = 0;
        for (int i = element_array[id].value_start; i <= element_array[id].value_end; i++)
        {
            out_buffer[count++] = value_array[i];
        }
    }
    return 0;
}

int next_id(int id)
{
    if (element_array)
    {
        if (id <= 0)
        {
            return 0;
        }
        ++id;
        if (nr_objects_store > id)
        {
            return id;
        }
    }
    return -1;
}

short get_type(int id)
{
    if (element_array)
    {
        if (nr_objects_store > id)
        {
            return element_array[id].type;
        }
    }
    return -1;
}

static void calculate_size(jsonman_element_t* element, uint* output_size, jsonman_print_t* type, uint* level)
{
}

static void serialize(jsonman_element_t* element, char* output, jsonman_print_t* type, uint* level)
{
}

char* jsonman_serialize(jsonman_element_t* root_element, jsonman_print_t print_type, uint* output_size)
{
    jsonman_last_error = JSONMAN_NO_ERROR;

    jsonman_element_t* structure = NULL;
    jsonman_print_t type;
    if (serialized_output)
    {
        jsonman_free_value(serialized_output);
    }


    uint size = 0;
    uint level = 0;
    calculate_size(structure, &size, &type, &level);
    serialized_output = jsonman_malloc((size + 1) * sizeof(char));
    serialized_output[size] = '\0';
    level = 0;
    serialize(structure, serialized_output, &type, &level);

    if (output_size)
    {
        *output_size = size;
    }
    return serialized_output;
}


jsonman_error_t jsonman_get_last_error()
{
    return jsonman_last_error;
}

int jsonman_get_error_pos()
{
    return jsonman_error_pos;
}
