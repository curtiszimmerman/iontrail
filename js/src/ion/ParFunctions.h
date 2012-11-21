/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99:
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jsion_par_functions_h__
#define jsion_par_functions_h__

#include "vm/threadpool.h"
#include "vm/forkjoin.h"
#include "gc/Heap.h"

namespace js {
namespace ion {

ForkJoinSlice *ParForkJoinSlice();
JSObject *ParNewGCThing(ForkJoinSlice *threadContext, gc::AllocKind allocKind, uint32_t thingSize);
bool ParWriteGuard(ForkJoinSlice *context, JSObject *object);
void ParBailout(uint32_t id);
bool ParCheckInterrupt(ForkJoinSlice *context);
bool ParExtendArray(HandleObject obj);

}
}

#endif
