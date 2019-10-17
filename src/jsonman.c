/**************************/
/*                        */
/*     Implementation     */
/*                        */
/**************************/

#include "jsonman.h"

#define RESET_NEXT  do {                                                      \
                        if (element_count < (*nr_objects))                    \
                        {                                                     \
                            ++element_count;                                  \
                            if (element_count < (*nr_objects))                \
                            {                                                 \
                                element_array[element_count].key_start = 0;   \
                                element_array[element_count].key_end = 0;     \
                                element_array[element_count].value_start = 0; \
                                element_array[element_count].value_end = 0;   \
                            }                                                 \
                        }                                                     \
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

static const char JM_COLON                = ':';
static const char JM_COMMA                = ',';
static const char JM_POINT                = '.';
static const char JM_DASH                 = '-';
static const char JM_DOUBLE_QUOTE         = '"';
static const char JM_BACKSLASH            = '\\';
static const char JM_CURLY_BRACKET_START  = '{';
static const char JM_SQUARE_BRACKET_START = '[';
static const char JM_CURLY_BRACKET_END    = '}';
static const char JM_SQUARE_BRACKET_END   = ']';
static const char JM_SPACE                = ' ';

const char JM_OBJECT         =  1;
const char JM_ARRAY          =  2;
const char JM_STRING         =  3;
const char JM_NUMBER         =  4;
const char JM_BOOLEAN        =  5;
const char JM_NAMED_OBJECT   =  6;
const char JM_NAMED_ARRAY    =  7;
const char JM_OBJECT_END     =  8;
const char JM_ARRAY_END      =  9;
const char JM_UNQUOTED_VALUE = 10;


#ifdef _WIN32
static const char* _NEW_LINE = "\r\n";
#else
static const char* _NEW_LINE = "\n";
#endif // _WIN32

static const short INDENT_SPACES = 4;


uint MALLOCS;
uint FREES;

jm_element_t* element_array = NULL;
char* value_array = NULL;
size_t nr_objects_store = 0;


static jm_error_t jm_last_error = JM_NO_ERROR;
static size_t jm_error_pos = 0;

static char* serialized_output = NULL;


static void* jm_malloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        jm_last_error = JM_ERROR_MEM_ALLOC;
        perror("");
    }
    else {
#ifdef JM_TEST
        ++MALLOCS;
#endif
    }
    return ptr;
}

static void free_value(void* ptr)
{
    if (ptr)
    {
        free(ptr);
    }
#ifdef JM_TEST
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
        if (i == *start && json[i] == JM_DASH)
        {
            continue;
        }
        if (isdigit(json[i]))
        {
            continue;
        }
        else if (json[i] == JM_POINT)
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

static void parse(char* json, size_t* nr_objects, size_t* values_size)
{
    if (*nr_objects == 0 && *values_size == 0)
    {
        jm_last_error = JM_ERROR_NO_DATA;
        return;
    }
    jm_free();
    element_array = jm_malloc((*nr_objects) * sizeof(jm_element_t));
    if (!element_array) return;

    value_array = jm_malloc((*values_size) + 1);
    if (!value_array) return;
    value_array[*values_size] = '\0';
    char stack[STACK_SIZE];

    int stackpos = -1;

    size_t len = strlen(json);
    size_t pos = 0;
    size_t value_array_pos = 0;
    char expect_new = 1;
    size_t element_count = 0;

    element_array[element_count].key_start = 0; 
    element_array[element_count].key_end = 0;
    element_array[element_count].value_start = 0;
    element_array[element_count].value_end = 0;

    while (pos < len)
    {
        while (isspace(json[pos])) ++pos;
        if (pos == len) break;

        if (stackpos == STACK_SIZE)
        {
            jm_last_error = JM_ERROR_STACK_OVERFLOW;
            return;
        }
        if (json[pos] == JM_COLON)  //between key and value
        {
            ++pos;
            while (isspace(json[pos])) ++pos;
            expect_new = 0;
        }
        else if (json[pos] == JM_COMMA)  //end of value
        {
            ++pos;
            while (isspace(json[pos])) ++pos;
            //check that comma is not followed by object_end or array_end
            if (json[pos] == JM_CURLY_BRACKET_END || json[pos] == JM_SQUARE_BRACKET_END)
            {
                jm_last_error = JM_ERROR_INVALID_INPUT;  //value not followed by comma when should
                jm_error_pos = pos;
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
            if (json[pos] != JM_CURLY_BRACKET_END && json[pos] != JM_SQUARE_BRACKET_END)
            {
                error = 1;
            }
            else if (json[pos] == JM_CURLY_BRACKET_END)
            {
                object_level = 1;
                array_level = 0;
            }
            else if (json[pos] == JM_SQUARE_BRACKET_END)
            {
                object_level = 0;
                array_level = 1;
            }
            if (!error)
            {
                short type;
                do
                {
                    size_t temp_pos = element_count - (++backwards_count);
                    if (temp_pos < 0)
                    {
                        break;
                    }
                    type = element_array[temp_pos].type;
                    switch (type) {
                    case JM_OBJECT:
                        --object_level;
                        break;
                    case JM_OBJECT_END:
                        ++object_level;
                        break;
                    case JM_ARRAY:
                        --array_level;
                        break;
                    case JM_ARRAY_END:
                        ++array_level;
                        break;
                    }
                } while ((object_level != 0 || array_level != 0) && (type != JM_OBJECT || type != JM_ARRAY));

                if (type == JM_OBJECT && json[pos] != JM_CURLY_BRACKET_END)
                {
                    error = 1;
                }
                else if (type == JM_ARRAY && json[pos] != JM_SQUARE_BRACKET_END)
                {
                    error = 1;
                }
            }
            if (error)
            {
                jm_last_error = JM_ERROR_INVALID_INPUT;
                jm_error_pos = pos;
                return;
            }
        }

        switch (json[pos])
        {
        case JM_CURLY_BRACKET_START:
        {
            element_array[element_count].type = JM_OBJECT;
            RESET_NEXT;
            ++pos;
            expect_new = 1;
            stack[++stackpos] = JM_OBJECT;
            break;
        }
        case JM_CURLY_BRACKET_END:
        {
            element_array[element_count].type = JM_OBJECT_END;
            RESET_NEXT;
            ++pos;
            expect_new = 1;
            stack[stackpos--] = 0;
            if (stack[stackpos] == JM_NAMED_OBJECT)
            {
                stack[stackpos--] = 0;
            }
            break;
        }
        case JM_SQUARE_BRACKET_START:
        {
            element_array[element_count].type = JM_ARRAY;
            RESET_NEXT;
            ++pos;
            expect_new = 1;
            stack[++stackpos] = JM_ARRAY;
            break;
        }
        case JM_SQUARE_BRACKET_END:
        {
            element_array[element_count].type = JM_ARRAY_END;
            RESET_NEXT;
            ++pos;
            expect_new = 1;
            stack[stackpos--] = 0;
            if (stack[stackpos] == JM_NAMED_ARRAY)
            {
                stack[stackpos--] = 0;
            }
            break;
        }
        default:
        {
            size_t temp_pos = pos;
            if (json[temp_pos] == JM_DOUBLE_QUOTE)  //Quoted key or value
            {
                ++temp_pos;
                int found = 0;
                while (!found)
                {
                    if (json[temp_pos] == JM_DOUBLE_QUOTE && json[temp_pos - 1] != JM_BACKSLASH)
                    {
                        found = 1;
                    }
                    ++temp_pos;
                }

                if (stack[stackpos] == JM_OBJECT || stack[stackpos] == JM_ARRAY)
                {
                    size_t next_char_at = temp_pos;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }

                    int increment_element_count = 1;
                    if (json[next_char_at] == JM_COLON)  //Named object / array or beginning of key/value element
                    {
                        ++next_char_at;
                        while (next_char_at < len && isspace(json[next_char_at])) {
                            ++next_char_at;
                        }
                        if (json[next_char_at] == JM_CURLY_BRACKET_START)
                        {
                            element_array[element_count].type = JM_NAMED_OBJECT;
                            stack[++stackpos] = JM_NAMED_OBJECT;
                        }
                        else if (json[next_char_at] == JM_SQUARE_BRACKET_START)
                        {
                            element_array[element_count].type = JM_NAMED_ARRAY;
                            stack[++stackpos] = JM_NAMED_ARRAY;
                        }
                        else if (json[next_char_at] == JM_DOUBLE_QUOTE)
                        {
                            element_array[element_count].type = JM_STRING;
                            stack[++stackpos] = JM_STRING;
                            increment_element_count = 0;
                        }
                        else
                        {
                            element_array[element_count].type = JM_UNQUOTED_VALUE;
                            stack[++stackpos] = JM_UNQUOTED_VALUE;
                            increment_element_count = 0;
                        }
                        SET_KEY_DATA(pos, temp_pos);
                    }
                    else if (stack[stackpos] == JM_ARRAY) //Array value
                    {
                        element_array[element_count].type = JM_STRING;
                        SET_VALUE_DATA(pos, temp_pos);
                    }
                    else //new key / value pair add to stack
                    {
                        element_array[element_count].type = JM_STRING;
                        SET_KEY_DATA(pos, temp_pos);
                        stack[++stackpos] = JM_STRING;
                        increment_element_count = 0;
                    }
                    if (increment_element_count)
                    {
                        RESET_NEXT;
                    }
                }
                else if (!expect_new) //String value, add value and pop from stack
                {
                    element_array[element_count].type = JM_STRING;
                    SET_VALUE_DATA(pos, temp_pos);
                    stack[stackpos--] = 0;

                    RESET_NEXT;

                    size_t next_char_at = temp_pos;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }
                    if (json[next_char_at] != JM_COMMA && json[next_char_at] != JM_CURLY_BRACKET_END && json[next_char_at] != JM_SQUARE_BRACKET_END)
                    {
                        jm_last_error = JM_ERROR_INVALID_INPUT;
                        jm_error_pos = next_char_at;
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
                    if (json[temp_pos] == JM_COLON
                        || json[temp_pos] == JM_COMMA
                        || json[temp_pos] == JM_CURLY_BRACKET_END
                        || json[temp_pos] == JM_SQUARE_BRACKET_END)
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
                    type = JM_NUMBER;
                }
                else if (is_boolean(json, &pos, &temp_pos))
                {
                    type = JM_BOOLEAN;
                }
                else
                {
                    type = JM_UNQUOTED_VALUE;
                }

                if (stack[stackpos] == JM_OBJECT || stack[stackpos] == JM_ARRAY) //Number or boolean array value, don't add to stack
                {
                    size_t next_char_at = temp_pos;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }

                    if (json[next_char_at] == JM_COLON)  //Named object / array (or unquoted key if allowed)
                    {
                        ++next_char_at;
                        while (next_char_at < len && isspace(json[next_char_at])) {
                            ++next_char_at;
                        }
                        if (json[next_char_at] == JM_CURLY_BRACKET_START)
                        {
                            element_array[element_count].type = JM_NAMED_OBJECT;
                            stack[++stackpos] = JM_NAMED_OBJECT;
                            RESET_NEXT;
                        }
                        else if (json[next_char_at] == JM_SQUARE_BRACKET_START)
                        {
                            element_array[element_count].type = JM_NAMED_ARRAY;
                            stack[++stackpos] = JM_NAMED_ARRAY;
                            RESET_NEXT;
                        }
#ifdef JM_ALLOW_UNQUOTED_JSON_KEYS
                        else
                        {
                            SET_KEY_DATA(pos, temp_pos);
                            stack[++stackpos] = JM_UNQUOTED_VALUE;  //unquoted key
                        }
#else
                        else
                        {
                            jm_last_error = JM_ERROR_INVALID_INPUT;
                            jm_error_pos = next_char_at;
                            return;
                        }
#endif // JM_ALLOW_UNQUOTED_JSON_KEYS
                    }
                    else if (stack[stackpos] == JM_ARRAY) //Array value
                    {
                        if (type == JM_UNQUOTED_VALUE)
                        {
                            jm_last_error = JM_ERROR_INVALID_INPUT;
                            jm_error_pos = next_char_at;
                            return;
                        }
                        element_array[element_count].type = type;
                        SET_VALUE_DATA(pos, temp_pos);
                        RESET_NEXT;

                    }
                    else { //new key / value pair add to stack
                        element_array[element_count].type = type;
                        SET_KEY_DATA(pos, temp_pos);
                        stack[++stackpos] = JM_UNQUOTED_VALUE;
                        RESET_NEXT;
                    }
                }
                else if (!expect_new) //Number or boolean value, add value and pop from stack
                {
                    element_array[element_count].type = type;
                    SET_VALUE_DATA(pos, temp_pos);
                    stack[stackpos--] = 0;
                    RESET_NEXT;

                    size_t next_char_at = temp_pos;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }
                    if (json[next_char_at] != JM_COMMA && json[next_char_at] != JM_CURLY_BRACKET_END && json[next_char_at] != JM_SQUARE_BRACKET_END)
                    {
                        jm_last_error = JM_ERROR_INVALID_INPUT;
                        jm_error_pos = next_char_at;
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

        if (json[pos] == JM_COLON)  //between key and value
        {
            ++pos;
            while (isspace(json[pos])) ++pos;
            expect_new = 0;
        }
        else if (json[pos] == JM_COMMA)  //end of value
        {
            ++pos;
            while (isspace(json[pos])) ++pos;
            expect_new = 1;
        }

        switch (json[pos])
        {
        case JM_CURLY_BRACKET_START:
        case JM_CURLY_BRACKET_END:
        case JM_SQUARE_BRACKET_START:
        case JM_SQUARE_BRACKET_END:
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
            if (json[temp_pos] == JM_DOUBLE_QUOTE)
            {
                ++temp_pos;
                int found = 0;
                while (!found)
                {
                    if (json[temp_pos] == JM_DOUBLE_QUOTE && json[temp_pos - 1] != JM_BACKSLASH)
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
                    if (json[temp_pos] == JM_COLON || json[temp_pos] == JM_COMMA || json[temp_pos] == JM_CURLY_BRACKET_END || json[temp_pos] == JM_SQUARE_BRACKET_END)
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

void jm_free()
{
    if (value_array)
    {
        free_value(value_array);
        value_array = NULL;
    }
    if (element_array)
    {
        free_value(element_array);
        element_array = NULL;
    }
    if (serialized_output)
    {
        free_value(serialized_output);
        serialized_output = NULL;
    }
}

int jm_parse(char* json)
{

#ifdef JM_TEST
    MALLOCS = 0;
    FREES = 0;
#endif // JM_TEST

    size_t nr_objects = 0;
    size_t values_size = 0;
    init_parse(json, &nr_objects, &values_size);
    parse(json, &nr_objects, &values_size);

    if (jm_last_error != JM_NO_ERROR)
    {
        return -1;
    }
    return 0;
}

int jm_get_key_length(int id, size_t* out_value)
{
    if (!out_value)
    {
        jm_last_error = JM_ERROR_OUT_PARAMETER_IS_NULL;
        return -1;
    }
    if (nr_objects_store <= id)
    {
        jm_last_error = JM_ERROR_INVALID_ID;
        return -1;
    }
    *out_value = (1 + element_array[id].key_end) - element_array[id].key_start;
    return 0;
}

int jm_get_value_length(int id, size_t* out_value)
{
    if (!out_value)
    {
        jm_last_error = JM_ERROR_OUT_PARAMETER_IS_NULL;
        return -1;
    }
    if (nr_objects_store <= id)
    {
        jm_last_error = JM_ERROR_INVALID_ID;
        return -1;
    }
    if (element_array[id].type == JM_OBJECT || element_array[id].type == JM_NAMED_OBJECT
        || element_array[id].type == JM_ARRAY || element_array[id].type == JM_NAMED_ARRAY)
    {
        jm_last_error = JM_ERROR_SIMPLE_VALUE_NOT_PRESENT;
        return -1;
    }
    *out_value = (1 + element_array[id].value_end) - element_array[id].value_start;
    return 0;
}

int jm_get_key(int id, char* out_buffer)
{
    if (!out_buffer)
    {
        jm_last_error = JM_ERROR_OUT_PARAMETER_IS_NULL;
        return -1;
    }
    if (nr_objects_store <= id)
    {
        jm_last_error = JM_ERROR_INVALID_ID;
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

int jm_get_value_as_string(int id, char* out_buffer)
{
    if (!out_buffer)
    {
        jm_last_error = JM_ERROR_OUT_PARAMETER_IS_NULL;
        return -1;
    }
    if (nr_objects_store <= id)
    {
        jm_last_error = JM_ERROR_INVALID_ID;
        return -1;
    }
    if (element_array[id].type == JM_OBJECT || element_array[id].type == JM_NAMED_OBJECT
        || element_array[id].type == JM_ARRAY || element_array[id].type == JM_NAMED_ARRAY)
    {
        jm_last_error = JM_ERROR_SIMPLE_VALUE_NOT_PRESENT;
        return -1;
    }

    size_t size = (1 + element_array[id].value_end) - element_array[id].value_start;
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

uint jm_new()
{
    return jm_last_error = JM_NO_ERROR;
}


int jm_next_id(int id)
{
    if (element_array)
    {
        if (id < 0)
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

short jm_get_type(int id)
{
    if (element_array)
    {
        if (nr_objects_store >= id)
        {
            return element_array[id].type;
        }
    }
    return -1;
}

static void calculate_size(jm_element_t* element, uint* output_size, jm_format_t* type, uint* level)
{
}

static void serialize(jm_element_t* element, char* output, jm_format_t* type, uint* level)
{
}

char* jm_serialize(jm_element_t* root_element, jm_format_t print_type, uint* output_size)
{
    jm_last_error = JM_NO_ERROR;

    jm_element_t* structure = NULL;
    jm_format_t type;
    if (serialized_output)
    {
        free_value(serialized_output);
    }


    uint size = 0;
    uint level = 0;
    calculate_size(structure, &size, &type, &level);
    serialized_output = jm_malloc((size + 1) * sizeof(char));
    serialized_output[size] = '\0';
    level = 0;
    serialize(structure, serialized_output, &type, &level);

    if (output_size)
    {
        *output_size = size;
    }
    return serialized_output;
}


jm_error_t jm_get_last_error()
{
    return jm_last_error;
}

int jm_get_error_pos()
{
    return jm_error_pos;
}
