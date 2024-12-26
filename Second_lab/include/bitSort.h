#ifndef BITSORT_H
#define BITSORT_H

void compare_and_swap(int *arr, int i, int j, int dir);
void bitonic_merge(int *arr, int low, int cnt, int dir);
void bitonic_sort(int *arr, int low, int cnt, int dir);
void bitsort(int *arr, int n, int up);

#endif BITSORT_H