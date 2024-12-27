#include <unistd.h>
#include <fcntl.h>

ssize_t stdin_read(void *buf, size_t cap)
{
	return read(STDIN_FILENO, buf, cap);
}

ssize_t stdout_write(const void *data, size_t len)
{
	return write(STDOUT_FILENO, data, len);
}

int mp_fopen(char* filename, int flags, mode_t mode){
	int res = open(filename, flags, O_RDWR);
	if(!res){
		res = open(filename, flags, O_CREAT);
	}
	return res;
}

int mp_fclose(int fd){
	return close(fd);
}