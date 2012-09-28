/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "base_cpp/nullable.h"
#include "base_cpp/array.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/d_bitset.h"
#include "base_cpp/obj.h"
#include "base_cpp/pool.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/ptr_pool.h"
#include "base_cpp/queue.h"
#include "base_cpp/red_black.h"

IMPL_EXCEPTION(indigo, NullableError, "Nullable");
IMPL_EXCEPTION(indigo, ArrayError, "array");
IMPL_EXCEPTION(indigo, AutoPtrError, "autoptr");
IMPL_EXCEPTION(indigo, DbitsetError, "Dynamic bitset");
IMPL_EXCEPTION(indigo, ObjError, "obj");
IMPL_EXCEPTION(indigo, PoolError, "pool");
IMPL_EXCEPTION(indigo, PtrArrayError, "ptr array");
IMPL_EXCEPTION(indigo, PtrPoolError, "ptr pool");
IMPL_EXCEPTION(indigo, QueueError, "queue");
IMPL_EXCEPTION(indigo, RedBlackTreeError, "red-black tree");
   