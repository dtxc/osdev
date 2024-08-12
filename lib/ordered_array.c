#include <mm/kheap.h>
#include <string.h>
#include <ordered_array.h>

char std_lt_predicate(type_t a, type_t b) {
    return (a < b) ? 1 : 0;
}

ordered_array_t new_oarr(uint32_t max, predicate_t p) {
    ordered_array_t res;
    res.arr = (void *) kmalloc(max * sizeof(type_t));
    memset(res.arr, 0, max * sizeof(type_t));
    res.size = 0;
    res.max = max;
    res.less_than = p;
    return res;
}

ordered_array_t place_oarr(void *addr, uint32_t max, predicate_t p) {
    ordered_array_t res;
    res.arr = (type_t *) addr;
    memset(res.arr, 0, max * sizeof(type_t));
    res.size = 0;
    res.max = max;
    res.less_than = p;
    return res;
}

void ins_oarr(type_t item, ordered_array_t *arr) {
    uint32_t iter = 0;

    while (iter < arr->size && arr->less_than(arr->arr[iter], item)) {
        iter++;
    }
    if (iter == arr->size) {
        arr->arr[arr->size++] = item;
    } else {
        type_t tmp = arr->arr[iter];
        arr->arr[iter] = item;
        while (iter < arr->size) {
            iter++;
            type_t tmp2 = arr->arr[iter];
            arr->arr[iter] = tmp;
            tmp = tmp2;
        }
        arr->size++;
    }
}

type_t lookup_oarr(uint32_t i, ordered_array_t *arr) {
    return arr->arr[i];
}

void rm_oarr(uint32_t i, ordered_array_t *arr) {
    while (i < arr->size) {
        arr->arr[i] = arr->arr[i+1];
        i++;
    }
    arr->size--;
}

void destroy_oarr(ordered_array_t *arr) {
    kfree(arr->arr);
}