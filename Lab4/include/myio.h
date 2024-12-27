#ifndef MYIO_H
#define MYIO_H

#include <unistd.h>

ssize_t my_write(char* str);
ssize_t my_read(char* buf);

int file_open(char* filename);

int write_to_file(int descriptor, char* str);

int file_close(int descriptor);

char* int_to_str(int number, char* string);



#endif