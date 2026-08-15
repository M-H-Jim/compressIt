#ifndef DSA_H
#define DSA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SWAP(a, b) do {         \
    __typeof__(a) temp = (a);   \
    (a) = (b);                  \
    (b) = temp;                 \
} while (0)

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

#define BYTE_COUNT 256


#define DEFINE_HEAP(type, name, compare)                                                    \
                                                                                            \
typedef struct name {                                                                       \
    type *data;                                                                             \
    size_t size;                                                                            \
    size_t capacity;                                                                        \
} name;                                                                                     \
                                                                                            \
name* create##name (void) {                                                                 \
    size_t capacity = BYTE_COUNT;                                                           \
    name *heap = (name *)malloc(sizeof(name));                                              \
    if (!heap) return NULL;                                                                 \
                                                                                            \
    heap->data = (type *)malloc(sizeof(type) * capacity);                                   \
    if (!heap->data) {                                                                      \
        free(heap);                                                                         \
        return NULL;                                                                        \
    }                                                                                       \
    heap->size = 0;                                                                         \
    heap->capacity = capacity;                                                              \
    return heap;                                                                            \
}                                                                                           \
                                                                                            \
void free##name (name *heap) {                                                              \
    if (!heap) return;                                                                      \
    free(heap->data);                                                                       \
    free(heap);                                                                             \
}                                                                                           \
                                                                                            \
bool empty##name (name *heap) {                                                             \
    return heap == NULL || heap->size == 0;                                                 \
}                                                                                           \
                                                                                            \
size_t size##name (name *heap) {                                                            \
    return heap ? heap->size : 0;                                                           \
}                                                                                           \
                                                                                            \
bool resize##name (name *heap, size_t newCapacity) {                                        \
    if (!heap) return false;                                                                \
    if (newCapacity > SIZE_MAX / sizeof(type)) return false;                                \
    type *newData = (type *)realloc(heap->data, sizeof(type) * newCapacity);                \
    if (!newData) return false;                                                             \
    heap->data = newData;                                                                   \
    heap->capacity = newCapacity;                                                           \
    return true;                                                                            \
}                                                                                           \
                                                                                            \
void siftUp##name (name *heap, size_t index) {                                              \
    while (index > 0) {                                                                     \
        size_t parent = (index - 1) / 2;                                                    \
        if (compare(&heap->data[index], &heap->data[parent]) <= 0) break;                   \
        SWAP(heap->data[index], heap->data[parent]);                                        \
        index = parent;                                                                     \
    }                                                                                       \
}                                                                                           \
                                                                                            \
void siftDown##name (name *heap, size_t index) {                                            \
    while (true) {                                                                          \
        size_t left  = 2 * index + 1;                                                       \
        size_t right = 2 * index + 2;                                                       \
        size_t best  = index;                                                               \
                                                                                            \
        if (left < heap->size && compare(&heap->data[left], &heap->data[best]) > 0)         \
            best = left;                                                                    \
        if (right < heap->size && compare(&heap->data[right], &heap->data[best]) > 0)       \
            best = right;                                                                   \
        if (best == index) break;                                                           \
                                                                                            \
        SWAP(heap->data[index], heap->data[best]);                                          \
        index = best;                                                                       \
    }                                                                                       \
}                                                                                           \
                                                                                            \
bool push##name (name *heap, type data) {                                                   \
    if (!heap) return false;                                                                \
    if (heap->size == heap->capacity) {                                                     \
        if (heap->capacity > SIZE_MAX / 2) return false;                                    \
        size_t newCapacity = 2 * heap->capacity;                                            \
        if (!resize##name(heap, newCapacity)) return false;                                 \
    }                                                                                       \
    heap->data[heap->size] = data;                                                          \
    siftUp##name(heap, heap->size);                                                         \
    heap->size++;                                                                           \
    return true;                                                                            \
}                                                                                           \
                                                                                            \
bool pop##name (name *heap, type *out) {                                                    \
    if (!heap || heap->size == 0) return false;                                             \
    if (out) *out = heap->data[0];                                                          \
    heap->size--;                                                                           \
    if (heap->size > 0) {                                                                   \
        heap->data[0] = heap->data[heap->size];                                             \
        siftDown##name(heap, 0);                                                            \
    }                                                                                       \
    return true;                                                                            \
}                                                                                           \
                                                                                            \
type peek##name (name *heap) {                                                              \
    if (!heap || heap->size == 0) return NULL;                                              \
    return heap->data[0];                                                                   \
}


#endif //DSA_H
