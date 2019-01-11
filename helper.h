#include<string.h>
#include<stdlib.h>
#ifndef _helper_h_

#define _helper_h_

char* UpdateString(char*, char, char);

int startsWith(char *pre, char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0 ? 1 : 0;
}

void Query_to_JSON(char query[])
{
    printf("\n%s\n", (char*)UpdateString(query, '&', ','));
}

char* UpdateString(char * input, char find, char replace)
{
    char * output = (char*)malloc(strlen(input));
    for (int i = 0; i < strlen(input); i++)
    {
        if (input[i] == find) output[i] = replace;
        else output[i] = input[i];
    }
    
    output[strlen(input)] = '\0';
    
    return output;
}

#endif
