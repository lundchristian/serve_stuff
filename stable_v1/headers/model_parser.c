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

            *out++ = c;
        }
    }
    *out = '\0';
    return 0;
}

/*
int key_val_parser(const char* key_val_str, struct model)
{
    if key == model attribute >> asign val to model attribute
}


*/






int main()
{
    const char* url_query = "note_title=Mons%2C+El+Perro&note_content=Mons+es+un+perro+muy+intelligente.%0D%0AUn+dia+Monsombre+nececita+un+sombrero%21";
    char* decoded_query = (char*)malloc(strlen(url_query) + 1);

    printf("%s\n", url_query);

    url_decoder(decoded_query, url_query);
    printf("%s\n", decoded_query);



}

