#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "mpio.h"

#define BUFSIZ 8192
#define SHMEM_SIZE 8192

#define SHM_NAME "/my_shm"
#define SEM_NAME "/my_sem"

int main(int argc, char* argv[]) {
    char file_name[BUFSIZ];

    if (argc < 2) {
        stdin_read(file_name, BUFSIZ);
    } else {
        strcpy(file_name, argv[1]);
    }

    char str[BUFSIZ];
    stdin_read(str, BUFSIZ);

    pid_t p;

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        myWrite("shm_open() error!\n");
        return -1;
    }

    if (ftruncate(shm_fd, SHMEM_SIZE) == -1) {
        myWrite("ftruncate() error!\n");
        return -1;
    }

    char* shm_buf = (char*)mmap(NULL, SHMEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_buf == MAP_FAILED) {
        myWrite("mmap() error!\n");
        return -1;
    }

    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        myWrite("sem_open() error!\n");
        return -1;
    }

    p = fork();
    if (p < 0) {
        myWrite("Fork failed!\n");
        return -1;
    } else if (p > 0) {
        // Parent process
        sem_wait(sem);
        myWrite(shm_buf);
        myWrite("\n");
        munmap(shm_buf, SHMEM_SIZE);
        close(shm_fd);
        shm_unlink(SHM_NAME);
        sem_close(sem);
        sem_unlink(SEM_NAME);
        return 0;
    } else {
        // Child process
        int ret = execl("./summ", "./summ", file_name, SHM_NAME, SEM_NAME, NULL);
        if (ret == -1) {
            char* sstr = strerror(errno);
            myWrite(sstr);
            return -1;
        }
    }

    munmap(shm_buf, SHMEM_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    sem_close(sem);
    sem_unlink(SEM_NAME);
    return 0;
}