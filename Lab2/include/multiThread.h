#ifndef BITONIC_SORT_H
#define BITONIC_SORT_H

#include <stddef.h>

/**
 * @brief Структура для передачи данных в потоки.
 */
struct PTHREAD_DATA {
    int *paddedValues;            /**< Указатель на массив с дополненными значениями. */
    unsigned int threadId;        /**< Идентификатор потока. */
    unsigned int chunkSize;       /**< Размер блока данных для каждого потока. */
    unsigned int mergeStep;       /**< Шаг слияния. */
    unsigned int bitonicSequenceSize; /**< Размер битoнической последовательности. */
};

/**
 * @brief Функция для обмена значениями между двумя переменными.
 * @param a Указатель на первую переменную.
 * @param b Указатель на вторую переменную.
 */
void swap(int *a, int *b);

/**
 * @brief Функция для реверса массива.
 * @param first Указатель на начало массива.
 * @param last Указатель на конец массива.
 */
void reverse(int* first, int* last);

/**
 * @brief Функция для сравнения и обмена элементов в массиве.
 * @param paddedValues Указатель на массив с дополненными значениями.
 * @param threadId Идентификатор потока.
 * @param chunkSize Размер блока данных для каждого потока.
 * @param mergeStep Шаг слияния.
 * @param bitonicSequenceSize Размер битoнической последовательности.
 */
void compareAndSwap(int paddedValues[], unsigned int threadId,
                    unsigned int chunkSize, unsigned int mergeStep, unsigned int bitonicSequenceSize);

/**
 * @brief Функция, выполняемая потоками.
 * @param arg Указатель на структуру PTHREAD_DATA.
 * @return NULL.
 */
void *thread_func(void* arg);

/**
 * @brief Основная функция для параллельной сортировки битoником.
 * @param values Указатель на массив значений для сортировки.
 * @param arrayLength Длина массива.
 * @param numThreads Количество потоков.
 * @param sortOrder Порядок сортировки: 1 - по возрастанию, 0 - по убыванию.
 */
void bitonicSort2(int values[], unsigned int arrayLength, unsigned int numThreads, int sortOrder);

#endif // BITONIC_SORT_H