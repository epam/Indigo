/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "base_cpp/array.h"
#include "base_cpp/cyclic_array.h"
#include "base_cpp/d_bitset.h"
#include "base_cpp/nullable.h"
#include "base_cpp/obj.h"
#include "base_cpp/pool.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/ptr_pool.h"
#include "base_cpp/queue.h"
#include "base_cpp/red_black.h"
#include <memory>

IMPL_EXCEPTION(indigo, NullableError, "Nullable");
IMPL_EXCEPTION(indigo, ArrayError, "array");
IMPL_EXCEPTION(indigo, CyclicArrayError, "cyclic array");
IMPL_EXCEPTION(indigo, DbitsetError, "Dynamic bitset");
IMPL_EXCEPTION(indigo, ObjError, "obj");
IMPL_EXCEPTION(indigo, PoolError, "pool");
IMPL_EXCEPTION(indigo, PtrArrayError, "ptr array");
IMPL_EXCEPTION(indigo, PtrPoolError, "ptr pool");
IMPL_EXCEPTION(indigo, QueueError, "queue");
IMPL_EXCEPTION(indigo, RedBlackTreeError, "red-black tree");
