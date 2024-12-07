#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZ 8192

ssize_t my_write(char* str){
    return write(STDOUT_FILENO, str, strlen(str));
}

ssize_t my_read(char* buf){
    return read(STDIN_FILENO, buf, BUFSIZ);
}

int file_open(char* filename){
    return open(filename, O_CREAT | O_TRUNC | O_RDWR , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP| S_IROTH | S_IWOTH);
}

int write_to_file(int descriptor, char* str){
    if(descriptor == -1){
        my_write("File error!\n");
        return 0;
    }
    write(descriptor, str, strlen(str));
    return 1;
}

int file_close(int descriptor){
    close(descriptor);
    return 0;
}
