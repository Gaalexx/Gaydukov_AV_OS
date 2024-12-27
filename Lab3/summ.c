#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "mpio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>

#define BUFSIZ 1024

int sum_of_strnums(char* str, int len) {
    char num[100];
    int n_len = 0;
    int res = 0;
    for (int i = 0; i < len; i++) {
        char c = str[i];
        if (isdigit(str[i]) || str[i] == '-') {
            num[n_len++] = str[i];
        } else if ((isspace(str[i]) && n_len != 0)) {
            num[n_len] = '\0';
            res += atoi(num);
            n_len = 0;
        } else if (iscntrl(str[i])) {
            num[n_len] = '\0';
            res += atoi(num);
            n_len = 0;
            break;
        } else {
            myWrite("Not a number detected!\n");
            myWrite(str + i);
            return 0;
        }
    }
    return res;
}

char* int_to_str(int number, char* string) {
    char rev_num[100];
    int len = 0;
    for (int i = 0; number != 0; i++) {
        rev_num[i] = '0' + number % 10;
        number /= 10;
        len = i + 1;
    }
    for (int i = len - 1, j = 0; i >= 0; i--, j++) {
        string[j] = rev_num[i];
    }
    string[len] = '\0';
    return string;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        myWrite("Wrong amount of arguments!\n");
        return -1;
    }

    char buf[BUFSIZ];
    int len = stdin_read(buf, BUFSIZ);
    buf[len] = '\0';

    char filename[100];
    strcpy(filename, argv[1]);

    int shm_fd = shm_open(argv[2], O_RDWR, 0666);
    if (shm_fd == -1) {
        myWrite("shm_open() error!\n");
        return -1;
    }

    char* shm_buf = (char*)mmap(NULL, BUFSIZ, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_buf == MAP_FAILED) {
        myWrite("mmap() error!\n");
        return -1;
    }

    sem_t* sem = sem_open(argv[3], 0);
    if (sem == SEM_FAILED) {
        myWrite("sem_open() error!\n");
        return -1;
    }

    int sum = sum_of_strnums(buf, strlen(buf) + 1);
    char res[100];
    int_to_str(sum, res);

    int dtr = open(filename, O_CREAT | O_TRUNC | O_RDWR , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP| S_IROTH | S_IWOTH);
    if (dtr == -1) {
        myWrite("File error!\n");
        return -1;
    }

    strcpy(shm_buf, res);
    munmap(shm_buf, BUFSIZ);
    close(shm_fd);

    write(dtr, res, strlen(res));
    write(dtr, "\n", 1);
    close(dtr);

    sem_post(sem);
    sem_close(sem);

    return 0;
}