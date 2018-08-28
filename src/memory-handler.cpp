/*
 * MIT License
 *
 * Copyright (c) 2018 Alexander Chumakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "memory-handler.h"

#include <cstring>
#include <assert.h>
#include <stddef.h>
#include <strings.h>
#include <QDebug>

#define MH_ITEMS_COUNT (8192)

#define MH_ITEMS_PER_FLAG (32)
#define MH_ITEMS_PER_BLOCK (224)

#define MH_FLAGS_COUNT (MH_ITEMS_COUNT/MH_ITEMS_PER_FLAG)
#define MH_FLAGS_ITEMS_PER_BLOCK (MH_ITEMS_PER_BLOCK/MH_ITEMS_PER_FLAG)
#define MH_COUNTERS_COUNT ((MH_ITEMS_COUNT + MH_ITEMS_PER_BLOCK - 1)/MH_ITEMS_PER_BLOCK)
#define MH_LAST_BLOCK_ITEMS_COUNT (MH_ITEMS_COUNT - MH_ITEMS_PER_BLOCK*(MH_COUNTERS_COUNT - 1))

/**
 * This is a static pool of items that will be allocated, released on user request
 */
static TData TheItems[MH_ITEMS_COUNT] = {{0, 0, NULL}};
/**
 * The bits in this array define if the corresponding item in 'TheItems' is free (0) or allocated (1).
 * The number of bits in this array is equal to the number of items in 'TheItems' pool.
 * The 1st item in 'TheFlags' describes the 1st 32 items in 'TheItems'.
 * The least significant bit defines the 1st item in  'TheItems'.
 */
static uint32_t TheFlags[MH_FLAGS_COUNT] = { UINT8_C(0) };
/**
 * This array defines the number of allocated items in sub-blocks in 'TheItems' of 32 items
 */
static uint8_t TheCounters[MH_COUNTERS_COUNT] = { UINT8_C(0) };

TData *alloc_new_block()
{
    // First, check if we have a block of 'MH_ITEMS_PER_BLOCK' items with at least 1 free item
    int freeBlock = -1;
    uint8_t *ptr = TheCounters;
    const uint8_t *const ptrEnd = TheCounters + MH_COUNTERS_COUNT - 1;
    while (ptr != ptrEnd) {
        if (*ptr != MH_ITEMS_PER_BLOCK) {
            // The current block of 'MH_ITEMS_PER_BLOCK' items has at least 1 free block
            freeBlock = (ptr - TheCounters);
            break;
        }

        ++ptr;
    }
    if (-1 == freeBlock) {
        // Now, check if the last block has free item(s)
        if (MH_LAST_BLOCK_ITEMS_COUNT != *ptrEnd) {
            // The last block still have some free items
            freeBlock = MH_COUNTERS_COUNT - 1;
        } else {
            // No free blocks found
            return NULL;
        }
    }

    // Find a free item now
    int freeItemIndex = -1;
    const int firstFlagsIndex = freeBlock*MH_FLAGS_ITEMS_PER_BLOCK;
    uint32_t *flagsPtr = TheFlags + firstFlagsIndex;
    const uint32_t *const flagsPtrEnd = flagsPtr + MH_FLAGS_ITEMS_PER_BLOCK;
    while (flagsPtr != flagsPtrEnd) {
        const uint32_t flags = *flagsPtr;
        if (flags != UINT32_C(0xffffffff)) {
            // Found some area of 'MH_ITEMS_PER_FLAG' items with at least 1 free item
            // Find the 1st free item
            const int bitIndex = ffs(~flags);
            // At least 1 item must be free, so,
            // 'bitIndex' must not be 0 (meaning a set bit is not-found)
            assert(bitIndex != 0);
            const int flagsIndex = (flagsPtr - TheFlags - firstFlagsIndex);
            freeItemIndex = MH_ITEMS_PER_BLOCK*freeBlock +
                            bitIndex - 1 +
                            flagsIndex*MH_ITEMS_PER_FLAG;
            assert(freeItemIndex < MH_ITEMS_COUNT);
            // Update the MemoryHandler state, allocate the items
            ++*ptr;
            *flagsPtr |= (1u << (bitIndex - 1));
            break;
        }

        ++flagsPtr;
    }

    assert(freeItemIndex >= 0 && freeItemIndex < MH_ITEMS_COUNT);
    return &TheItems[freeItemIndex];
}

void free_block(TData *ptr)
{
    const int itemIndex = ptr - TheItems;
    if (itemIndex < 0 || itemIndex >= MH_ITEMS_COUNT) {
        // Wrong pointer
        qCritical() << "Wrong pointer:" << ptr;
        return;
    }

    uint8_t &counter = TheCounters[itemIndex/MH_ITEMS_PER_BLOCK];
    if (counter) {
        --counter;
    } else {
        qCritical() << "Wrong pointer: releasing item in a free block:" << ptr;
        return;
    }

    // Clear the 'item-allocated' flag
    TheFlags[itemIndex/MH_ITEMS_PER_FLAG] &= ~(1u << (itemIndex & (MH_ITEMS_PER_FLAG - 1)));
    // Clear the released item
    memset(&TheItems[itemIndex], 0, sizeof (*TheItems));
}
