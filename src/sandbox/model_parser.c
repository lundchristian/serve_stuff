#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Credit to 'chmike' from StackOverflow*/
int url_decoder(char* out, const char* in)
{
    static const char tbl[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
         0, 1, 2, 3, 4, 5, 6, 7,  8, 9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
    };

    char c, v1, v2, *beg = out;

    if(in != NULL)
    {
        while((c = *in++) != '\0')
        {
            if(c == '%')
            {
                if((v1 = tbl[(unsigned char)*in++]) < 0 || (v2 = tbl[(unsigned char)*in++]) < 0)
                {
                    *beg = '\0';
                    return -1;
                }
                c = (v1 << 4) | v2;
            }

            if (c == '+') c = ' ';
            if (c == '\r') c = ' ';
            if (c == '\n') c = ' ';

            *out++ = c;
        }
    }
    *out = '\0';
    return 0;
}

typedef struct data_model data_model;

struct data_model {
    int id;
    char* file_name;
    void (*insert)(data_model* self, const char* key_val_str);
    char* (*fetch)(data_model* self, int id);
};

void insert_impl(data_model* self, const char* key_val_str)
{
    const char* file_name = self->file_name;
    FILE* file = fopen(file_name, "a+");
    if (file == NULL)
    {
        printf("Failed to open file %s\n", file_name);
        return;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        int id;
        if (sscanf(line, "%*[^=]=%*[^&]&%*[^=]=%*[^&]&%d", &id) == 1)
        {
            if (id > self->id)
            {
                self->id = id;
            }
        }
    }
    rewind(file);
    
    self->id++;
    fprintf(file, "id=%d&%s\n", self->id, key_val_str);
    
    fclose(file);
}

char* fetch_impl(data_model* self, int id)
{
    const char* file_name = self->file_name;
    FILE* file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Failed to open file %s\n", file_name);
        return NULL;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        int line_id;
        if (sscanf(line, "id=%d", &line_id) == 1 && line_id == id)
        {
            fclose(file);
            return strdup(line);
        }
    }
    
    fclose(file);
    return NULL;
}

void data_model_ctor(data_model *self, const char *file_name)
{
    size_t file_name_len = strlen(file_name) + 1;
    self->file_name = (char*)malloc(file_name_len);
    strcpy(self->file_name, file_name);

    self->id = 0;
    self->insert = insert_impl;
    self->fetch = fetch_impl;
}

void data_model_dtor(data_model *self)
{
    free(self->file_name);
}

void run_test()
{
    const char* note_query_1 = "note_title=Shopping+List&note_content=Agurk%2C+Tandkrem%2C+Mer";
    const char* note_query_2 = "note_title=Todo&note_content=Løpe%2C+Hoppe%2C+Vaske%2C+Drikke";

    const char* person_query_1 = "name=Christian+Lund&age=25&mail=lundchristian%40me.com";
    const char* person_query_2 = "name=Mons+Bal&age=1&mail=monsobal%40dog.com";

    const char* car_query_1 = "color=Blue&top_speed=350&tyres=Michelin&price=4500000";
    const char* car_query_2 = "color=Red&top_speed=100&tyres=F1&price=25999";
    // char* note_query_1_decoded = (char*)malloc(strlen(note_query_1) + 1);
    // char* note_query_2_decoded = (char*)malloc(strlen(note_query_2) + 1);

    // char* person_query_1_decoded = (char*)malloc(strlen(person_query_1) + 1);
    // char* person_query_2_decoded = (char*)malloc(strlen(person_query_2) + 1);

    // char* car_query_1_decoded = (char*)malloc(strlen(car_query_1) + 1);
    // char* car_query_2_decoded = (char*)malloc(strlen(car_query_2) + 1);

    // url_decoder(note_query_1_decoded, note_query_1);
    // printf("Decoded\n%s\n", note_query_1_decoded);
    // url_decoder(note_query_2_decoded, note_query_2);
    // printf("Decoded\n%s\n", note_query_2_decoded);

    // url_decoder(person_query_1_decoded, person_query_1);
    // printf("Decoded\n%s\n", person_query_1_decoded);
    // url_decoder(person_query_2_decoded, person_query_2);
    // printf("Decoded\n%s\n", person_query_2_decoded);

    // url_decoder(car_query_1_decoded, car_query_1);
    // printf("Decoded\n%s\n", car_query_1_decoded);
    // url_decoder(car_query_2_decoded, car_query_2);
    // printf("Decoded\n%s\n", car_query_2_decoded);

    // data_model db_note, db_person, db_car;
    // data_model_ctor(&db_note, "note.txt");
    // data_model_ctor(&db_person, "person.txt");
    // data_model_ctor(&db_car, "car.txt");

    // db_note.insert(&db_note, note_query_1_decoded);
    // db_note.insert(&db_note, note_query_2_decoded);

    // db_person.insert(&db_person, person_query_1_decoded);
    // db_person.insert(&db_person, person_query_2_decoded);

    // db_car.insert(&db_car, car_query_1_decoded);
    // db_car.insert(&db_car, car_query_2_decoded);

    // char* note_1 = db_note.fetch(&db_note, 1);
    // printf("Fetched\n%s\n", note_1);
    // char* person_1 = db_person.fetch(&db_person, 2);
    // printf("Fetched\n%s\n", person_1);
    // char* car_1 = db_car.fetch(&db_car, 1);
    // printf("Fetched\n%s\n", car_1);
}



    /*Open file from file_path*/
    /*Look for handle: {{}}*/
    /*Compare content of handle with key from key_val*/
    /*If the content == key => inject val*/
    /*Else inject not found*/
    /*Return file content*/

    /* Example
    1. call handle_inject("output.html", "id=2&name=Mons Bal&age=1&mail=monsobal@dog.com")
    2. Open output.html

    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>output</title>
    </head>
    <body>
        <center>
            <p>{{name}}</p>
        </center>
    </body>
    </html>

    3. Replace handle

    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>output</title>
    </head>
    <body>
        <center>
            <p>Mons Bal</p>
        </center>
    </body>
    </html>

    4. Return file content
    */
char* handle_inject(const char* file_content, const char* key_val)
{
    char* handle_start = strstr(file_content, "{{");
    handle_start += 2;
    char* handle_end = strstr(handle_start, "}}");
    if (handle_start == 0 || handle_end == 0)
    {
        printf("Error: Handle not found");
        exit(1);
    }

    size_t len = handle_end - handle_start;
    char* html_key = (char*)malloc(len + 1);
    if (html_key == NULL) return NULL;
    strncpy(html_key, handle_start, len);
    html_key[len] = '\0';

    char* str_key = strstr(key_val, html_key);
    if (str_key == 0)
    {
        printf("Error: Key not found");
        exit(1);
    }

    str_key += len + 1;
    char* str_val = strstr(str_key, "&");

    size_t len_val = str_val - str_key;
    char* replace = (char*)malloc(len_val + 1);
    if (replace == NULL) return NULL;
    strncpy(replace, str_key, len_val);
    replace[len_val] = '\0';

    size_t output_len = strlen(file_content) - len - 4 + strlen(replace);
    char* output_content = (char*)malloc(output_len + 1);

    return replace;
}

void test_handle_inject()
{
    const char* file_content = 
                            "<!DOCTYPE html>"
                            "<html lang=\"en\">"
                            "<head>"
                                "<meta charset=\"UTF-8\">"
                                "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"
                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                                "<title>output</title>"
                            "</head>"
                            "<body>"
                                "<center>"
                                    "<p>{{name}}</p>"
                                "</center>"
                            "</body>"
                            "</html>";
    const char* key_val = "id=2&name=Mons Bal&age=1&mail=monsobal@dog.com";
    printf("%s\n", handle_inject(file_content, key_val));
}

int main()
{
    test_handle_inject();
}