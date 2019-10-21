/**************************/
/*                        */
/*     Implementation     */
/*                        */
/**************************/

#include "jsonman.h"

#define NO_ERROR jm_last_error = JM_NO_ERROR

#define RESET_NEXT  do {                                                        \
                        if (element_count < (*nr_objects))                      \
                        {                                                       \
                            ++element_count;                                    \
                            if (element_count < (*nr_objects))                  \
                            {                                                   \
                                element_array[element_count].type        = 0;   \
                                element_array[element_count].level       = 0;  \
                                element_array[element_count].key_start   = 0;   \
                                element_array[element_count].key_end     = 0;   \
                                element_array[element_count].value_start = 0;   \
                                element_array[element_count].value_end   = 0;   \
                            }                                                   \
                        }                                                       \
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

static jm_element_t* element_array = NULL;
static char* value_array = NULL;
static size_t nr_objects_store = 0;


static jm_error_t jm_last_error = JM_NO_ERROR;
static size_t jm_error_pos = 0;
static jm_user_defined_t* jm_p_user_defined;
static size_t user_defined_id = 0;
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

static void free_user_defined()
{
    if (jm_p_user_defined)
    {
        if (jm_p_user_defined->next)
        {
            free_user_defined(jm_p_user_defined->next);
        }
        free_value(jm_p_user_defined);
    }
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
    size_t element_count = 0;

    element_array[element_count].type = 0;
    element_array[element_count].level = 0;
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
        }

        switch (json[pos])
        {
        case JM_CURLY_BRACKET_START:
        {
            element_array[element_count].type = JM_OBJECT;
            stack[++stackpos] = JM_OBJECT;
            if (stackpos > 0)
            {
                if (stack[stackpos - 1] != JM_NAMED_OBJECT)
                {
                    element_array[element_count].level = stackpos;
                }
                else
                {
                    element_array[element_count].level = stackpos - 1;
                }
            }
            RESET_NEXT;
            ++pos;
            break;
        }
        case JM_CURLY_BRACKET_END:
        {
            element_array[element_count].type = JM_OBJECT_END;
            stack[stackpos--] = 0;
            if (stack[stackpos] == JM_NAMED_OBJECT)
            {
                stack[stackpos--] = 0;
            }
            RESET_NEXT;
            ++pos;
            break;
        }
        case JM_SQUARE_BRACKET_START:
        {
            element_array[element_count].type = JM_ARRAY;
            stack[++stackpos] = JM_ARRAY;
            if (stackpos > 0)
            {
                if (stack[stackpos - 1] != JM_NAMED_ARRAY)
                {
                    element_array[element_count].level = stackpos;
                }
                else
                {
                    element_array[element_count].level = stackpos - 1;
                }
            }
            RESET_NEXT;
            ++pos;
            break;
        }
        case JM_SQUARE_BRACKET_END:
        {
            element_array[element_count].type = JM_ARRAY_END;
            stack[stackpos--] = 0;
            if (stack[stackpos] == JM_NAMED_ARRAY)
            {
                stack[stackpos--] = 0;
            }
            RESET_NEXT;
            ++pos;
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

                int is_key = 0;
                char value_type_if_key = 0;
                size_t next_char_at = temp_pos;
                while (next_char_at < len && isspace(json[next_char_at])) {
                    ++next_char_at;
                }

                if (json[next_char_at] == JM_COLON)
                {
                    is_key = 1;
                    ++next_char_at;
                    while (next_char_at < len && isspace(json[next_char_at])) {
                        ++next_char_at;
                    }
                    switch (json[next_char_at])
                    {
                    case JM_DOUBLE_QUOTE:
                        value_type_if_key = JM_STRING;
                        break;
                    case JM_CURLY_BRACKET_START:
                        value_type_if_key = JM_NAMED_OBJECT;
                        break;
                    case JM_SQUARE_BRACKET_START:
                        value_type_if_key = JM_NAMED_ARRAY;
                        break;
                    default:
                        value_type_if_key = JM_UNQUOTED_VALUE;
                    }
                }
                else if (json[next_char_at] != JM_COMMA && json[next_char_at] != JM_CURLY_BRACKET_END && json[next_char_at] != JM_SQUARE_BRACKET_END)
                {
                    jm_last_error = JM_ERROR_INVALID_INPUT;
                    jm_error_pos = next_char_at;
                    return;
                }

                if (is_key)
                {
                    SET_KEY_DATA(pos, temp_pos);
                    if (value_type_if_key == JM_NAMED_OBJECT || value_type_if_key == JM_NAMED_ARRAY)
                    {
                        stack[++stackpos] = value_type_if_key;
                        element_array[element_count].type = value_type_if_key;
                        element_array[element_count].level = stackpos;
                        RESET_NEXT;
                    }
                }
                else
                {
                    SET_VALUE_DATA(pos, temp_pos);
                    element_array[element_count].type = JM_STRING;
                    RESET_NEXT;
                }
            }
            else  //Unquoted string / value
            {
                ++temp_pos;
                int found = 0;
                int is_key = 0;
                char value_type_if_key = 0;
                while (!found)
                {
                    if (json[temp_pos] == JM_COLON
                        || json[temp_pos] == JM_COMMA
                        || json[temp_pos] == JM_CURLY_BRACKET_END
                        || json[temp_pos] == JM_SQUARE_BRACKET_END)
                    {
                        if (json[temp_pos] == JM_COLON)
                        {
                            is_key = 1;
                            size_t next_char_at = temp_pos + 1;
                            while (next_char_at < len && isspace(json[next_char_at])) {
                                ++next_char_at;
                            }
                            switch (json[next_char_at])
                            {
                            case JM_DOUBLE_QUOTE:
                                value_type_if_key = JM_STRING;
                                break;
                            case JM_CURLY_BRACKET_START:
                                value_type_if_key = JM_NAMED_OBJECT;
                                break;
                            case JM_SQUARE_BRACKET_START:
                                value_type_if_key = JM_NAMED_ARRAY;
                                break;
                            default:
                                value_type_if_key = JM_UNQUOTED_VALUE;
                            }
                        }
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

                if (is_key)
                {
#ifndef JM_ALLOW_UNQUOTED_JSON_KEYS
                    jm_last_error = JM_ERROR_INVALID_INPUT;
                    jm_error_pos = pos;
                    return;
#endif
                    SET_KEY_DATA(pos, temp_pos);
                    if (value_type_if_key == JM_NAMED_OBJECT || value_type_if_key == JM_NAMED_ARRAY)
                    {
                        stack[++stackpos] = value_type_if_key;
                        element_array[element_count].type = value_type_if_key;
                        element_array[element_count].level = stackpos;
                        RESET_NEXT;
                    }
                }
                else
                {
                    if (is_number(json, &pos, &temp_pos))
                    {
                        element_array[element_count].type = JM_NUMBER;
                    }
                    else if (is_boolean(json, &pos, &temp_pos))
                    {
                        element_array[element_count].type = JM_BOOLEAN;
                    }
                    else
                    {
                        element_array[element_count].type = JM_UNQUOTED_VALUE;
                    }
                    SET_VALUE_DATA(pos, temp_pos);
                    //stack[stackpos--] = 0;
                    RESET_NEXT;
                }
            }
            size_t length = temp_pos - pos;
            pos += length;
        }
        }
    }
    if (stackpos != -1) 
    {
        jm_last_error = JM_ERROR_INVALID_INPUT;
        jm_error_pos = -1;
        return;
    }
    user_defined_id = *nr_objects;
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


static int validate_out_param(void* p)
{
    NO_ERROR;
    if (!p)
    {
        jm_last_error = JM_ERROR_INVALID_OUT_PARAMETER;
        return 0;
    }
    return 1;
}
static int validate_id(size_t* id, size_t* out_id)
{
    NO_ERROR;
    if (!id || nr_objects_store <= *id)
    {
        jm_last_error = JM_ERROR_INVALID_ID;
        return 0;
    }
    if (!validate_out_param(out_id))
    {
        return 0;
    }
    return 1;
}

static int validate_id_and_buffer(size_t* id, char* out)
{
    NO_ERROR;
    if (!id || nr_objects_store <= *id)
    {
        jm_last_error = JM_ERROR_INVALID_ID;
        return 0;
    }
    if (!validate_out_param(out))
    {
        return 0;
    }
    return 1;
}

static int validate_root()
{
    NO_ERROR;
    if (!jm_p_user_defined)
    {
        jm_last_error = JM_ERROR_NO_ROOT;
        return 0;
    }
    return 1;
}

static int validate_type(const char type)
{
    NO_ERROR;
    if (type >= 1 && type <= 10)
    {
        return 1;
    }
    jm_last_error = JM_ERROR_INVALID_TYPE;
    return 0;
}


static int find_unnamed_element(const char type, size_t* from_id, size_t* out_value, int* level)
{
    if (!element_array || nr_objects_store <= 0)
    {
        jm_last_error = JM_ERROR_NO_DATA;
        return -1;
    }
    if (!validate_id(from_id, out_value))
    {
        return -1;
    }

    int id = *from_id < 0 ? 0 : *from_id;

    for (int i = id; i < nr_objects_store; i++)
    {
        if (element_array[i].type == type)
        {
            if (*level >= 0 && element_array[i].level == *level)
            {
                *out_value = i;
                return 0;
            }
            else if (*level < 0)
            {
                *out_value = i;
                return 0;
            }
        }
    }
    jm_last_error = JM_ERROR_ELEMENT_NOT_FOUND;
    return -1;
}

static int find_named_element(const char type, size_t* from_id, size_t* out_value, int* level, char* key_in, char* value_in)
{
    if (!element_array || nr_objects_store <= 0)
    {
        jm_last_error = JM_ERROR_NO_DATA;
        return -1;
    }
    if (!validate_id(from_id, out_value))
    {
        return -1;
    }

    int id = *from_id < 0 ? 0 : *from_id;

    for (int i = id; i < nr_objects_store; i++)
    {
        if (element_array[i].type == type)
        {
            if (*level >= 0)
            {
                if (type == JM_NAMED_OBJECT || type == JM_NAMED_ARRAY)  //level is stored on these, but not on simple types
                {
                    if (*level != element_array[i].level)
                    {
                        continue;
                    }
                    *out_value = i;
                    return 0;
                }
                else
                {
                    //Find parent structure
                    int count = i;
                    int temp_level = 0;
                    do 
                    {
                        --count;
                        if (element_array[count].type == JM_OBJECT_END || element_array[count].type == JM_ARRAY_END)
                        {
                            temp_level++;
                        }
                        else if (element_array[count].type == JM_OBJECT || element_array[count].type == JM_ARRAY)
                        {
                            temp_level--;
                        }
                    } while ((element_array[count].type != JM_OBJECT && element_array[count].type != JM_ARRAY) || temp_level != -1);

                    if (element_array[count].level != *level)
                    {
                        continue;
                    }
                }
            }
            int match = 1;
            if (key_in)
            {
                size_t key_length = (1 + element_array[i].key_end) - element_array[i].key_start;
                if (strlen(key_in) != key_length)
                {
                    match = 0;
                }
                else
                {
                    char key[key_length + 1];
                    key[key_length] = '\0';
                    jm_get_key(i, key);
                    if (strcmp(key, key_in) != 0)
                    {
                        match = 0;
                    }
                }
            }
            if (value_in && match)
            {
                size_t value_length = (1 + element_array[i].value_end) - element_array[i].value_start;
                if (strlen(value_in) != value_length)
                {
                    match = 0;
                }
                else
                {
                    char value[value_length + 1];
                    value[value_length] = '\0';
                    jm_get_value_as_string(i, value);
                    if (strcmp(value, value_in) != 0)
                    {
                        match = 0;
                    }
                }
            }
            if (match)
            {
                *out_value = i;
                return 0;
            }
        }
    }
    jm_last_error = JM_ERROR_ELEMENT_NOT_FOUND;
    return -1;
}

static int new_root(const char type, size_t* id_out)
{
    if (validate_out_param(id_out))
    {
        return -1;
    }
    jm_free();
    jm_p_user_defined = jm_malloc(sizeof(jm_user_defined_t));
    if (!jm_p_user_defined)
    {
        return -1;
    }
    jm_p_user_defined->id = user_defined_id++;
    jm_p_user_defined->type = type;
    *id_out = jm_p_user_defined->id;
    return 0;
}

static int new_element(const char type, jm_user_defined_operation_t operation, size_t parent_id, char* key, char* value, size_t* id_out)
{
    if (!validate_out_param(id_out))
    {
        return -1;
    }
    if (nr_objects_store <= 0 && !validate_root())
    {
        return -1;
    }
    if (!validate_type(type))
    {
        return -1;
    }
    jm_user_defined_t* t = jm_p_user_defined;
    while (t->next)
    {
        t = t->next;
    }
    t->next = jm_malloc(sizeof(jm_user_defined_t));
    t->type = type;
    t->flags = t->flags | operation;
    t->parent_id = parent_id;
    if (key)
    {
        size_t len = strlen(key);
        t->key = jm_malloc(len + 1);
        memcpy(t->key, key, len);
    }
    if (value)
    {
        size_t len = strlen(value);
        t->value = jm_malloc(len + 1);
        memcpy(t->value, value, len);
    }
    t->id = user_defined_id++;
    *id_out = t->id;
    return 0;
}

static jm_user_defined_t* get_user_defined(size_t* id)
{
    if (jm_p_user_defined)
    {
        if (jm_p_user_defined->parent_id == *id && ((jm_p_user_defined->flags & DELETE) > 0))
        {
            return NULL;
        }
        jm_user_defined_t* ptr = jm_p_user_defined->next;
        while (ptr)
        {
            if (ptr->parent_id == *id && ((ptr->flags & DELETE) > 0))
            {
                return NULL;
            }
            else if (ptr->parent_id == *id)
            {
                return ptr;
            }
            ptr = ptr->next;
        }
    }
    return NULL;
}

static int delete_object_or_array(size_t* id, uint user_defined, uint is_object)
{
    char start_type;
    char end_type;
    if (is_object)
    {
        start_type = JM_OBJECT;
        end_type = JM_OBJECT_END;
    }
    else
    {
        start_type = JM_ARRAY;
        end_type = JM_ARRAY_END;
    }
    if (user_defined)
    {
        jm_user_defined_t* user_defined = get_user_defined(id);
        size_t level = -1;
        while (level != 0)
        {
            user_defined = user_defined->next;
            if (user_defined)
            {
                if (user_defined->type == start_type)
                {
                    --level;
                }
                else if (user_defined->type == end_type)
                {
                    ++level;
                }
                user_defined->flags = user_defined->flags | DELETE;
            }
        }
    }
    else
    {
        if (id > 0 && element_array[*id - 1].type == JM_NAMED_ARRAY && ((element_array[*id - 1].flags & DELETE) == 0))
        {
            element_array[*id - 1].flags = element_array[*id - 1].flags | DELETE;
        }
        element_array[*id].flags = element_array[*id].flags | DELETE;
        size_t level = -1;
        size_t count = *id;
        while (level != 0)
        {
            ++count;
            if (element_array[count].type == start_type)
            {
                --level;
            }
            else if (element_array[count].type == end_type)
            {
                ++level;
            }
            element_array[count].flags = element_array[count].flags | DELETE;
        }
    }
    return 0;
}

static int delete_named_object_or_named_array(size_t* id, uint is_user_defined, uint is_named_object)
{
    if (is_user_defined)
    {
        jm_user_defined_t* user_defined = get_user_defined(id);
        if (!user_defined)
        {
            return -1;
        }
        user_defined->flags = user_defined->flags | DELETE;
        if (is_named_object)
        {
            if (user_defined->next && user_defined->next->type == JM_OBJECT)
            {
                return delete_object_or_array(user_defined->next->id, 1, 1);
            }
        }
        else
        {
            if (user_defined->next && user_defined->next->type == JM_ARRAY)
            {
                return delete_object_or_array(user_defined->next->id, 1, 0);
            }
        }
    }
    else
    {
        element_array[*id].flags = element_array[*id].flags | DELETE;
        if (((*id) + 1) < nr_objects_store)
        {
            if (is_named_object)
            {
                if (element_array[(*id) + 1].type == JM_OBJECT)
                {
                    return delete_object_or_array((*id) + 1, 0, 1);
                }
            }
            else
            {
                if (element_array[(*id) + 1].type == JM_ARRAY)
                {
                    return delete_object_or_array((*id) + 1, 0, 0);
                }
            }
        }
    }
    return 0;
}

static int delete_simple_type(size_t* id, uint is_user_defined)
{
    if (is_user_defined)
    {
        jm_user_defined_t* user_defined = get_user_defined(id);
        if (!user_defined)
        {
            return -1;
        }
        user_defined->flags = user_defined->flags | DELETE;
    }
    else
    {
        element_array[*id].flags = element_array[*id].flags | DELETE;
    }
    return 0;
}


//************************* Public functions below *************************************

void jm_free()
{
    NO_ERROR;
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
    if (jm_p_user_defined)
    {
        free_user_defined(jm_p_user_defined);
    }
    user_defined_id = 0;
}

int jm_parse(char* json)
{

#ifdef JM_TEST
    MALLOCS = 0;
    FREES = 0;
#endif // JM_TEST

    NO_ERROR;
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

int jm_get_key_length(size_t id, size_t* out_value)
{
    if (!validate_id(&id, out_value))
    {
        return -1;
    }

    *out_value = (1 + element_array[id].key_end) - element_array[id].key_start;
    return 0;
}

int jm_get_value_length(size_t id, size_t* out_value)
{
    if (!validate_id(&id, out_value))
    {
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

int jm_get_key(size_t id, char* out_buffer)
{
    if (!validate_id_and_buffer(&id, out_buffer))
    {
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

int jm_get_value_as_string(size_t id, char* out_buffer)
{
    if (!validate_id_and_buffer(&id, out_buffer))
    {
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

int jm_find_next_object(size_t from_id, size_t* id_found, int level)
{
    return find_unnamed_element(JM_OBJECT, &from_id, id_found, &level);
}

int jm_find_next_named_object(size_t from_id, size_t* id_found, int level, char* name)
{
    return find_named_element(JM_NAMED_OBJECT, &from_id, id_found, &level, name, NULL);
}

int jm_find_next_array(size_t from_id, size_t* id_found, int level)
{
    return find_unnamed_element(JM_ARRAY, &from_id, id_found, &level);
}

int jm_find_next_named_array(size_t from_id, size_t* id_found, int level, char* name)
{
    return find_named_element(JM_NAMED_ARRAY, &from_id, id_found, &level, name, NULL);
}

int jm_find_next_number(size_t from_id, size_t* id_found, int level, char* name, char* value)
{
    return find_named_element(JM_NUMBER, &from_id, id_found, &level, name, value);
}

int jm_find_next_boolean(size_t from_id, size_t* id_found, int level, char* name, int* value)
{
    if (value)
    {
        if (*value)
        {
            return find_named_element(JM_BOOLEAN, &from_id, id_found, &level, name, "true");
        }
        else
        {
            return find_named_element(JM_BOOLEAN, &from_id, id_found, &level, name, "false");
        }
    }
    return find_named_element(JM_BOOLEAN, &from_id, id_found, &level, name, NULL);
}

int jm_find_next_string(size_t from_id, size_t* id_found, int level, char* name, char* value)
{
    return find_named_element(JM_STRING, &from_id, id_found, &level, name, value);
}

int jm_new_root_object(size_t* id)
{
    return new_root(JM_OBJECT, id);
}

int jm_new_root_array(size_t* id)
{
    return new_root(JM_ARRAY, id);
}

int jm_add_object(size_t id, size_t* id_out)
{
    if (id < nr_objects_store)
    {
        if (element_array[id].type != JM_OBJECT &&
            element_array[id].type != JM_NAMED_OBJECT &&
            element_array[id].type != JM_ARRAY &&
            element_array[id].type != JM_NAMED_ARRAY)
        {
            jm_last_error = JM_ERROR_PARENT_ELEMENT_WRONG_TYPE;
            return -1;
        }
        new_element(JM_OBJECT, CREATE, id, NULL, NULL, id_out);
        return 0;
    }
    else if (jm_p_user_defined)
    {
        jm_user_defined_t* ptr = jm_p_user_defined;
        while (ptr->next)
        {   
            if (id == ptr->id)
            {
                if (ptr->type != JM_OBJECT &&
                    ptr->type != JM_NAMED_OBJECT &&
                    ptr->type != JM_ARRAY &&
                    ptr->type != JM_NAMED_ARRAY)
                {
                    jm_last_error = JM_ERROR_PARENT_ELEMENT_WRONG_TYPE;
                    return -1;
                }
                new_element(JM_OBJECT, CREATE, id, NULL, NULL, id_out);
                return 0;
            }
            ptr = ptr->next;
        }
    }
    jm_last_error = JM_ERROR_ELEMENT_NOT_FOUND;
    return -1;
}

int jm_add_array(size_t parent_id, size_t* id_out)
{
    return 0;
}

int jm_add_named_object(size_t parent_id, char* name, size_t* id_out)
{
    return 0;
}

int jm_add_named_array(size_t parent_id, char* name, size_t* id_out)
{
    return 0;
}

int jm_add_number(size_t parent_id, char* name, char* number, size_t* id_out)
{
    return 0;
}

int jm_add_boolean(size_t parent_id, char* name, uint value, size_t* id_out)
{
    return 0;
}

int jm_add_string(size_t parent_id, char* name, char* value, size_t* id_out)
{
    return 0;
}

int delete_element(size_t id)
{
    if (element_array && id < nr_objects_store)
    {
        if ((element_array[id].flags & DELETE) > 0)
        {
            jm_last_error = JM_ERROR_ELEMENT_NOT_FOUND;
            return -1;
        }

        if (element_array[id].type == JM_OBJECT) return delete_object_or_array(&id, 0, 1);
        else if (element_array[id].type == JM_ARRAY) return delete_object_or_array(&id, 0, 0);
        else if (element_array[id].type == JM_NAMED_OBJECT) return delete_named_object_or_named_array(&id, 0, 1);
        else if (element_array[id].type == JM_NAMED_ARRAY) return delete_named_object_or_named_array(&id, 0, 0);
        else return delete_simple_type(&id, 0);
    }
    else
    {
        jm_user_defined_t* ptr = get_user_defined(id);
        if (!ptr)
        {
            jm_last_error = JM_ERROR_ELEMENT_NOT_FOUND;
            return -1;
        }

        if (ptr->type == JM_OBJECT) return delete_object_or_array(&id, 1, 1);
        else if (ptr->type == JM_ARRAY) return delete_object_or_array(&id, 1, 0);
        else if (ptr->type == JM_NAMED_OBJECT) return delete_named_object_or_named_array(&id, 1, 1);
        else if (ptr->type == JM_NAMED_ARRAY) return delete_named_object_or_named_array(&id, 1, 0);
        else return delete_simple_type(&id, 1);
    }
}



int jm_next_id(int id)
{
    NO_ERROR;
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
    NO_ERROR;
    if (element_array)
    {
        if (nr_objects_store >= id)
        {
            return element_array[id].type;
        }
    }
    return -1;
}



int jm_calculate_size(jm_format_t type, size_t* output_size)
{
    if (!validate_out_param(output_size))
    {
        return -1;
    }
    if (element_array)
    {

    }
    else
    {

    }

    return 0;
}

int jm_serialize(jm_format_t type, char* output)
{
    NO_ERROR;

    /*
    jm_element_t* structure = NULL;
    if (serialized_output)
    {
        free_value(serialized_output);
    }


    uint size = 0;
    uint level = 0;
    //jm_calculate_size(structure, &size, &type, &level);
    serialized_output = jm_malloc((size + 1) * sizeof(char));
    serialized_output[size] = '\0';
    level = 0;
    //serialize(structure, serialized_output, &type, &level);

    if (output_size)
    {
        *output_size = size;
    }
    */
    return 0;
}


jm_error_t jm_get_last_error()
{
    return jm_last_error;
}

int jm_get_error_pos()
{
    return jm_error_pos;
}
