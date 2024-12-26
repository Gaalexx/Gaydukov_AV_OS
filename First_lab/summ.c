#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <ctype.h>
#include "mpio.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZ 1024

int sum_of_strnums(char* str, int len){
    char num[100];
    int n_len = 0;
    int res = 0;
    for (int i = 0; i < len; i++)
    {
        char c = str[i];
        if(isdigit(str[i]) || str[i] == '-'){
            num[n_len++] = str[i];
        }
        else if((isspace(str[i]) && n_len != 0)){
            num[n_len] = '\0';
            res += atoi(num);
            n_len = 0;
        }
        else if(iscntrl(str[i])){
            num[n_len] = '\0';
            res += atoi(num);
            n_len = 0;
            break;
        }
        else{
            stdout_write("Not a number detected!\n", 23);
            stdout_write(str + i, 1);
            return 0;
        }
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
    for (int i = len - 1, j = 0; i >= 0; i--, j++)
    {
        string[j] = rev_num[i];
    }
    string[len] = '\0';
    return string;
}

int main(int argc, char* argv[]){
    if(argc < 2){
        stdout_write("Wrong amount of arguments!\n", 27);
        return -1;
    }
    char buf[BUFSIZ];

    int len = stdin_read(buf, BUFSIZ);
    buf[len] = '\0';

    char filename[100];
    strcpy(filename, argv[1]);
    int sum = sum_of_strnums(buf, strlen(buf) + 1);
    char res[100];
    int_to_str(sum, (char*)res);
    int dtr = open(filename, O_CREAT | O_TRUNC | O_RDWR , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP| S_IROTH | S_IWOTH);
    if(dtr == -1){
        stdout_write("File error!\n", 12);
        return -1;
    }
    write(dtr, res, strlen(res));
    write(dtr, "\n", 1);

    close(dtr);

    stdout_write(res, strlen(res) + 1);
    return 0;
    //res надо записать в файл с именем filename. Через fp2 надо передать результат обратно
}