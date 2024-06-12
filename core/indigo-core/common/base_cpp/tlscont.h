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

#ifndef __tlscont_h__
#define __tlscont_h__

#include <memory>
#include <stack>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <safe_ptr.h>

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/pool.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/red_black.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    // Session identifiers manager.
    // Every thread have local session ID that corresponds to the all
    // local session variables.
    class DLLEXPORT _SIDManager
    {
    public:
        static _SIDManager& getInst();

        void setSessionId(qword id);
        qword getSessionId() const;

        qword allocSessionId();
        // Add specified SID to the vacant list.
        // This method should be called before thread exit if SID was
        // assigned automatically (not by manual TL_SET_SESSION_ID call)
        void releaseSessionId(qword id);

        DECL_ERROR;

    private:
        _SIDManager() = default;

        // Thread local key for storing current session ID
        static qword& _sessionId();

        std::atomic<qword> lastNewSID;
    };

// Macros for managing session IDs for current thread
#define TL_GET_SESSION_ID() _SIDManager::getInst().getSessionId()
#define TL_SET_SESSION_ID(id) _SIDManager::getInst().setSessionId(id)
#define TL_ALLOC_SESSION_ID() _SIDManager::getInst().allocSessionId()
#define TL_RELEASE_SESSION_ID(id) _SIDManager::getInst().releaseSessionId(id)

    // Container that keeps one instance of specified type per session
    template <typename T>
    class _SessionLocalContainer
    {
    public:
        T& createOrGetLocalCopy(const qword id = TL_GET_SESSION_ID())
        {
            auto map = sf::xlock_safe_ptr(_map);
            if (!map->count(id))
            {
                map->emplace(id, std::make_unique<T>());
            }
            return *map->at(id);
        }

        // FIXME:MK: it's not thread safe, decide what to do
        T& getLocalCopy(const qword id = TL_GET_SESSION_ID()) const
        {
            const auto map = sf::slock_safe_ptr(_map);
            return *map->at(id);
        }

        void removeLocalCopy(const qword id = TL_GET_SESSION_ID())
        {
            auto map = sf::xlock_safe_ptr(_map);
            map->erase(id);
        }

        bool hasLocalCopy(const qword id = TL_GET_SESSION_ID()) const
        {
            const auto map = sf::slock_safe_ptr(_map);
            return map->count(id) > 0;
        }

    private:
        sf::safe_shared_hide_obj<std::unordered_map<qword, std::unique_ptr<T>>> _map;
    };

// Macros for working with global variables per each session
// By tradition this macros start with TL_, but should start with SL_
#define TL_DECL(type, name) static _SessionLocalContainer<type> TLSCONT_##name
#define TL_GET(type, name) type& name = (TLSCONT_##name).createOrGetLocalCopy()
#define TL_GET_BY_ID(type, name, id) type& name = (TLSCONT_##name).createOrGetLocalCopy(id)
#define TL_DEF(className, type, name) _SessionLocalContainer<type> className::TLSCONT_##name
}

// "Quasi-static" variable definition. Calls clear() at the end
// #define QS_DEF(TYPE, name) \
//    static _ReusableVariablesPool<_GET_TYPE(TYPE)> _POOL_##name;                                                                          \
//    int _POOL_##name##_idx; \
//    _GET_TYPE(TYPE)& name = _POOL_##name.getVacant(_POOL_##name##_idx); \
//    _ReusableVariablesAutoRelease<_GET_TYPE(TYPE)> _POOL_##name##_auto_release; \
//    _POOL_##name##_auto_release.init(_POOL_##name##_idx, &_POOL_##name);                                                                                  \
//    name.clear();
// Use this for debug purposes if you suspect QS_DEF in something bad
#define QS_DEF(TYPE, name) TYPE name;

// "Quasi-static" variable definition. Calls clear_resize() at the end
// #define QS_DEF_RES(TYPE, name, len) \
//    static _ReusableVariablesPool<_GET_TYPE(TYPE)> _POOL_##name;                                                                          \
//    int _POOL_##name##_idx; \
//    _GET_TYPE(TYPE)& name = _POOL_##name.getVacant(_POOL_##name##_idx); \
//    _ReusableVariablesAutoRelease<_GET_TYPE(TYPE)> _POOL_##name##_auto_release; \
//    _POOL_##name##_auto_release.init(_POOL_##name##_idx, &_POOL_##name);                                                                                  \
//    name.clear_resize(len);
// Use this for debug purposes if you suspect QS_DEF in something bad
#define QS_DEF_RES(TYPE, name, len)                                                                                                                            \
    TYPE name;                                                                                                                                                 \
    name.clear_resize(len);

// Reusable class members definition
// By tradition this macros start with TL_, but should start with SL_
// To work with them you should first define commom pool with CP_DECL,
// then define it in the source class with CP_DEF(cls), and initialize
// in the constructor via CP_INIT before any TL_CP_ initializations
//

// Add this to class definition
#define TL_CP_DECL(TYPE, name) TYPE name

// Add this to constructor initialization list
#define TL_CP_GET(name) name()

#define CP_DECL bool _cp_decl_pseudo_init;

#define CP_INIT _cp_decl_pseudo_init(true)

#define CP_DEF(cls)

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __tlscont_h__
