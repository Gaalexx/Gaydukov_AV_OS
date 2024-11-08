#include "mpio.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>
extern int errno;  


#define BUFSIZ 8192


int main(int argc, char* argv[]){
    char file_name[BUFSIZ];
    if(argc < 2){
        stdin_read(file_name, BUFSIZ);
    }else{
        strcpy(file_name, argv[1]);
    }

    char str[BUFSIZ];
    stdin_read(str, BUFSIZ);

    int fd1[2], fd2[2];
    pid_t p;

    if(pipe(fd1) == -1 || pipe(fd2) == -1) {
        stdout_write("Pipe failed!\n", 13);
        return -1;
    }

    p = fork();

    if(p < 0){
        stdout_write("Fork failed!\n", 13);
        return -1;
    }
    else if(p > 0){
        //главный процесс 

        char res[100];

        close(fd1[0]);

        write(fd1[1], str, strlen(str) + 1);
        close(fd1[1]);
        wait(NULL);
        close(fd2[1]);

        int len = read(fd2[0], res, 100);
        res[len] = '\n';
        res[len+1] = 0;

        stdout_write(res, strlen(res) + 1);
        stdout_write("\n", 1);
        close(fd2[0]);
    }
    else{
        //дочерний процесс, отсюда нужно запустить следующую программу с передачей аругментов 
        // + не забыть про filename первым аргументом
        /* close(STDOUT_FILENO);
        dup(fd2[1]); */
        dup2(fd2[1], STDOUT_FILENO);
        dup2(fd1[0], STDIN_FILENO);

        /* close(STDIN_FILENO);
        dup(fd1[0]); */

        close(fd1[0]);
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);

        int ret = execl("./summ", "./summ", file_name, NULL);
        if(ret == -1){
            char* sstr = strerror(errno);
            
            stdout_write(sstr, strlen(sstr) + 1);
            return -1;
        }
    }

    //free(finstr);
    return 0;
}