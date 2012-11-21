/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jsarrayinlines_h___
#define jsarrayinlines_h___

#include "jsinferinlines.h"
#include "jsobjinlines.h"

inline void
JSObject::markDenseArrayNotPacked(JSContext *cx)
{
    JS_ASSERT(isDenseArray());
    MarkTypeObjectFlags(cx, this, js::types::OBJECT_FLAG_NON_PACKED_ARRAY);
}

inline void
JSObject::initializeDenseArrayElements(uint32_t &objInitLen,
                                       uint32_t newInitLength)
{
    JSCompartment *comp = compartment();
    size_t offset = objInitLen;
    for (js::HeapSlot *sp = elements + objInitLen;
         sp != elements + newInitLength;
         sp++, offset++)
        sp->init(comp, this, offset, js::MagicValue(JS_ARRAY_HOLE));
    objInitLen = newInitLength;
}

inline JSObject::EnsureDenseResult
JSObject::extendDenseArray(js::Allocator *alloc, uint32_t extra)
{
    // NB: This function can execute either in sequential or parallel mode.

    JS_ASSERT(isDenseArray());
    JS_ASSERT(!js_PrototypeHasIndexedProperties(this));
    JS_ASSERT(getDenseArrayCapacity() == getElementsHeader()->initializedLength);

    uint32_t &initlen = getElementsHeader()->initializedLength;
    uint32_t requiredCapacity = initlen + extra;
    if (requiredCapacity < initlen)
        return ED_SPARSE;
    if (requiredCapacity > MIN_SPARSE_INDEX
        && willBeSparseDenseArray(requiredCapacity, extra))
        return ED_SPARSE;
    if (!growElements(alloc, requiredCapacity))
        return ED_FAILED;
    initializeDenseArrayElements(initlen, initlen + extra);
    return ED_OK;
}

inline void
JSObject::ensureDenseArrayInitializedLength(JSContext *cx, uint32_t index, uint32_t extra)
{
    /*
     * Ensure that the array's contents have been initialized up to index, and
     * mark the elements through 'index + extra' as initialized in preparation
     * for a write.
     */
    JS_ASSERT(index + extra <= getDenseArrayCapacity());
    uint32_t &initlen = getElementsHeader()->initializedLength;
    if (initlen < index)
        markDenseArrayNotPacked(cx);
    if (initlen < index + extra)
        initializeDenseArrayElements(initlen, index + extra);
}

inline JSObject::EnsureDenseResult
JSObject::ensureDenseArrayElements(JSContext *cx, unsigned index, unsigned extra)
{
    JS_ASSERT(isDenseArray());

    unsigned currentCapacity = getDenseArrayCapacity();

    unsigned requiredCapacity;
    if (extra == 1) {
        /* Optimize for the common case. */
        if (index < currentCapacity) {
            ensureDenseArrayInitializedLength(cx, index, 1);
            return ED_OK;
        }
        requiredCapacity = index + 1;
        if (requiredCapacity == 0) {
            /* Overflow. */
            return ED_SPARSE;
        }
    } else {
        requiredCapacity = index + extra;
        if (requiredCapacity < index) {
            /* Overflow. */
            return ED_SPARSE;
        }
        if (requiredCapacity <= currentCapacity) {
            ensureDenseArrayInitializedLength(cx, index, extra);
            return ED_OK;
        }
    }

    /*
     * We use the extra argument also as a hint about number of non-hole
     * elements to be inserted.
     */
    if (requiredCapacity > MIN_SPARSE_INDEX &&
        willBeSparseDenseArray(requiredCapacity, extra)) {
        return ED_SPARSE;
    }
    if (!growElements(cx, requiredCapacity))
        return ED_FAILED;

    ensureDenseArrayInitializedLength(cx, index, extra);
    return ED_OK;
}

#endif /* jsarrayinlines_h___ */
