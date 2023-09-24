
// AW library
// Aarni Gratseff (aarni.gratseff@gmail.com)
// Sorting

#ifndef AW_SORT_H
#define AW_SORT_H

#include <stdint.h>

#define AW_SORT_VALUE_BITS 32
#define AW_SORT_NUM_INDEX_BITS 16
#define AW_SORT_INDEX_BITS_POSITION (AW_SORT_VALUE_BITS - AW_SORT_NUM_INDEX_BITS)
#define AW_SORT_ORDER_MAX (((uint32_t)1 << AW_SORT_INDEX_BITS_POSITION) - 1)

#define AW_SORT_ORDER(a, b) \
    ((indices[a] & AW_SORT_ORDER_MAX) > (indices[b] & AW_SORT_ORDER_MAX))

#define AW_SORT_NORDER(a, b) \
    ((indices[a] & AW_SORT_ORDER_MAX) < (indices[b] & AW_SORT_ORDER_MAX))

#define AW_SORT_SWAP(a, b) swap32(indices[a], indices[b])

// Define the Leonardo numbers
static inline int32_t leonardo(int32_t k) {
    if (k < 2) {
        return 1;
    }

    return leonardo(k - 1) + leonardo(k - 2) + 1;
}

// Build the Leonardo heap by merging
// pairs of adjacent trees
static inline void heapify(uint32_t __far* indices, int32_t start, int32_t end) {
    int32_t i = start;
    int32_t j = 0;
    int32_t k = 0;

    while (k < end - start + 1) {
        if (k & 0xAAAAAAAA) {
            j = j + i;
            i = i >> 1;
        } else {
            i = i + j;
            j = j >> 1;
        }

        k++;
    }

    while (i > 0) {
        j = j >> 1;
        k = i + j;
        while (k < end) {
            if (AW_SORT_NORDER(k, k - i)) {
                break;
            }

            AW_SORT_SWAP(k, k - i);
            k = k + i;
        }

        i = j;
    }
}

static inline void smoothsort(uint32_t __far* indices, uint16_t n) {
    int32_t p, q, r;
    uint16_t i;

    if (n == 0) {
        return;
    }

    p = n - 1;
    q = p;
    r = 0;

    // Build the Leonardo heap by merging
    // pairs of adjacent trees
    while (p > 0) {
        if ((r & 0x03) == 0) {
            heapify(indices, r, q);
        }

        if (leonardo(r) == p) {
            r = r + 1;
        } else {
            r = r - 1;
            q = q - leonardo(r);
            heapify(indices, r, q);
            q = r - 1;
            r = r + 1;
        }

        AW_SORT_SWAP(0, p);
        p--;
    }

    // Convert the Leonardo heap
    // back into an array
    for (i = 0; i < n - 1; i++) {
        uint16_t j = i + 1;
        while (j > 0 && AW_SORT_ORDER(j, j - 1)) {
            AW_SORT_SWAP(j, j - 1);
            j--;
        }
    }
}

#endif // AW_SORT_H
