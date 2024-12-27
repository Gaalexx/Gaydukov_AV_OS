#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>

#include "intstr.h"
#include "myio.h"


int hasOnlyNums(char* num){
    for (size_t i = 0; i < strlen(num) && num[i] != '\n'; i++)
    {
        if(!isdigit(num[i]) || (num[i] == '-' && i != 0)){
            return 0;
        }
    }
    return 1;
}

char* double_to_str(double num, char* str){
    int integer_num = (int)num;
    str = int_to_str(integer_num, str);
    num -= integer_num;
    num *= 100000000;
    integer_num = (int)num;

    char rev_num[100];
    int len = 0;
    for (int i = 0; integer_num != 0; i++)
    {
        rev_num[i] = '0' + integer_num % 10;
        integer_num /= 10;
        len = i + 1;
    }

    char* tmp;
    tmp = (char*)realloc(str, sizeof(char) * (sizeof(str) + len + 1));
    if(tmp == NULL){
        free(str);
        return NULL;
    }
    
    str = tmp;
    str[strlen(str)] = '.';
    int stln = strlen(str);
    for (int i = len - 1; i >= 0; i--, stln++)
    {
        str[stln] = rev_num[i];
    }
    str[stln] = '\0';
    return str;
}

int str_to_int(char* num){
    int res = 0, beg = 0;
    if(!hasOnlyNums(num)){
        return 0;
    }
    else if(num[0] == '-'){
        ++beg;
    }
    for (int i = beg; i < strlen(num) && num[i] != '\n'; i++)
    {
        res = (res * 10) + num[i] - '0';
    }
    return res;
}

char* int_to_str(int number, char* string){
    char rev_num[100];
    int len = 0;
    for (int i = 0; number != 0; i++)
    {
        rev_num[i] = '0' + number % 10;
        number /= 10;
        len = i + 1;
    }
    string = (char*)malloc(sizeof(char) * (len + 1));
    if(string == NULL){
        return " ";
    }
    for (int i = len - 1, j = 0; i >= 0; i--, j++)
    {
        string[j] = rev_num[i];
    }
    string[len] = '\0';
    return string;
}

void print_int(const int num){
    char* c = int_to_str(num, c);
    my_write(c);
    free(c);
    return;
}

void print_double(const double num){
    char *c = double_to_str(num, c);
    my_write(c);
    free(c);
    return;
}
