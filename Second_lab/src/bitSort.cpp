#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


void compare_and_swap(int *arr, int i, int j, int dir) {
    if (dir == (arr[i] > arr[j])) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void bitonic_merge(int *arr, int low, int cnt, int dir) {
    if (cnt > 1) {
        for (int i = low; i < low + cnt / 2; i++) {
            compare_and_swap(arr, i, i + cnt / 2, dir);
        }
        bitonic_merge(arr, low, cnt / 2, dir);
        bitonic_merge(arr, low + cnt / 2, cnt / 2, dir);
    }
}

void bitonic_sort(int *arr, int low, int cnt, int dir) {
    if (cnt > 1) {
        bitonic_sort(arr, low, cnt / 2, 1);  
        bitonic_sort(arr, low + cnt / 2, cnt / 2, 0);  
        bitonic_merge(arr, low, cnt, dir);  
    }
}

void bitsort(int *arr, int n, int up) {
    bitonic_sort(arr, 0, n, up);
}