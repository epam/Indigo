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

#ifndef __item_h__
#define __item_h__

#include <map>
#include <memory>
#include <stdint.h>
#include <vadefs.h>

namespace indigo2
{
    using namespace std;

    typedef ID int;
    typedef Handle uintptr_t;

    typedef Item shared_ptr<_Item>;
    class _Item : public Serializable
    {
        ID id;
        std::string name;
        std::mutex lock;
        Context ctx;
        virtual void _scan_members(std::istream* is, std::ostream* os, const std::string& name)
        {
            SERIALIZE_FIELD(id)
            SERIALIZE_FIELD(name)
        }

    };

    typedef Context shared_ptr<_Context>;
    class _Context : public _Item
    {
        bool option1;
        int oprion2;
        static Item newItem();
    };

    class ExportedItem
    {
        static std::mutex lock;
        static map<void*, ExportedItem item> map;
        shared_ptr<void> item;
        shared_ptr<void> parent;
        Handle getHandle(void* item, void* parent)
        {
            const std::lock_guard<std::mutex> lk(lock);
            map[item.ptr()] = {item, parent};
            return item.ptr();
        }
        template <T> T getItem(Handle handle)
        {
            return (T)map[handle].item;
        }
        void dropItem(Handle handle)
        {
            const std::lock_guard<std::mutex> lk(lock);
            return map.remove(handle);
        }
    };
} // namespace indigo2
#endif
