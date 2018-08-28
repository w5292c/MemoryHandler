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

#include "memoryhandlertest.h"

#include "memory-handler.h"

#include <QtTest/QtTest>

void MemoryHandlerTest::test()
{
    int i;
    TData *item = NULL;
    TData *firstItem = NULL;
    for (i = 0; i < 16384; ++i) {
        item = alloc_new_block();
        if (!firstItem) {
            firstItem = item;
        }
        if (i < 8192) {
            QVERIFY(item != NULL);
        } else {
            QVERIFY(item == NULL);
        }
    }

    free_block(firstItem);
    item = alloc_new_block();
    QCOMPARE(item, firstItem);
    item = alloc_new_block();
    QCOMPARE(item, (TData *)NULL);

    free_block(firstItem + 1234);
    item = alloc_new_block();
    QCOMPARE(item, firstItem + 1234);
    item = alloc_new_block();
    QCOMPARE(item, (TData *)NULL);

    free_block(firstItem + 8190);
    item = alloc_new_block();
    QCOMPARE(item, firstItem + 8190);
    item = alloc_new_block();
    QCOMPARE(item, (TData *)NULL);

    free_block(firstItem + 8192);
    item = alloc_new_block();
    QCOMPARE(item, (TData *)NULL);

    qDebug() << "Test data:" << (TData *)0x100f - (TData *)0x1000 << ", size:" << sizeof (TData *);
}
