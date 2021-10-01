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
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "base_c/defs.h"
#include "base_c/os_tls.h"
#include "base_cpp/array.h"
#include <memory>
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
        static _SIDManager& getInst(void);
        static std::mutex& getLock();

        _SIDManager(void);
        ~_SIDManager(void);

        void setSessionId(qword id);
        qword allocSessionId(void);
        qword getSessionId(void);
        // Add specified SID to the vacant list.
        // This method should be called before thread exit if SID was
        // assigned automatically (not by manual TL_SET_SESSION_ID call)
        void releaseSessionId(qword id);

        DECL_ERROR;

    private:
        qword* _getID() const;

        // Thread local key for storing current session ID
        TLS_IDX_TYPE _tlsIdx;
        RedBlackSet<qword> _allSIDs;
        qword _lastNewSID;
        // Array with vacant SIDs
        Array<qword> _vacantSIDs;
        std::vector<qword*> _pIds;
    };

// Macros for managing session IDs for current thread
#define TL_GET_SESSION_ID() _SIDManager::getInst().getSessionId()
#define TL_SET_SESSION_ID(id) _SIDManager::getInst().setSessionId(id)
#define TL_ALLOC_SESSION_ID() _SIDManager::getInst().allocSessionId()
#define TL_RELEASE_SESSION_ID(id) _SIDManager::getInst().releaseSessionId(id)

    // Container that keeps one instance of specified type per session
    template <typename T> class _SessionLocalContainer
    {
    public:
        T& getLocalCopy()
        {
            return getLocalCopy(TL_GET_SESSION_ID());
        }

        T& getLocalCopy(const qword id)
        {
            std::lock_guard<std::mutex> locker(_lock);
            if (!_map.count(id))
            {
                _map[id] = std::make_unique<T>();
            }
            return *_map.at(id);
        }

        void removeLocalCopy()
        {
            removeLocalCopy(TL_GET_SESSION_ID());
        }

        void removeLocalCopy(const qword id)
        {
            std::lock_guard<std::mutex> locker(_lock);
            if (_map.count(id))
            {
                _map.erase(id);
            }
        }

    private:
        std::unordered_map<qword, std::unique_ptr<T>> _map;
        std::mutex _lock;
    };

    // Helpful templates to deal with commas in template type names
    // to be able to write like
    // QS_DEF((std::unordered_map<std::string, int>), atoms_id);
    // See http://stackoverflow.com/a/13842784
    template <typename T> struct ArgumentType;
    template <typename T, typename U> struct ArgumentType<T(U)>
    {
        typedef U Type;
    };
#define _GET_TYPE(t) ArgumentType<void(t)>::Type

// Macros for working with global variables per each session
// By tradition this macros start with TL_, but should start with SL_
#define TL_DECL_EXT(type, name) extern _SessionLocalContainer<_GET_TYPE(type)> TLSCONT_##name
#define TL_DECL(type, name) static _SessionLocalContainer<_GET_TYPE(type)> TLSCONT_##name
#define TL_GET(type, name) _GET_TYPE(type)& name = (TLSCONT_##name).getLocalCopy()
#define TL_DECL_GET(type, name)                                                                                                                                \
    TL_DECL(type, name);                                                                                                                                       \
    TL_GET(type, name)
#define TL_GET2(type, name, realname) _GET_TYPE(type)& name = (TLSCONT_##realname).getLocalCopy()
#define TL_GET_BY_ID(type, name, id) _GET_TYPE(type)& name = (TLSCONT_##name).getLocalCopy(id)
#define TL_DEF(className, type, name) _SessionLocalContainer<_GET_TYPE(type)> className::TLSCONT_##name
#define TL_DEF_EXT(type, name) _SessionLocalContainer<_GET_TYPE(type)> TLSCONT_##name
} // namespace indigo

// "Quasi-static" variable definition. Calls clear() at the end
//#define QS_DEF(TYPE, name)                                                                                                                                     \
//    static _ReusableVariablesPool<_GET_TYPE(TYPE)> _POOL_##name;                                                                          \
//    int _POOL_##name##_idx;                                                                                                                                    \
//    _GET_TYPE(TYPE)& name = _POOL_##name.getVacant(_POOL_##name##_idx);                                                                                       \
//    _ReusableVariablesAutoRelease<_GET_TYPE(TYPE)> _POOL_##name##_auto_release;                                                                                \
//    _POOL_##name##_auto_release.init(_POOL_##name##_idx, &_POOL_##name);                                                                                  \
//    name.clear();
// Use this for debug purposes if you suspect QS_DEF in something bad
#define QS_DEF(TYPE, name) TYPE name;

// "Quasi-static" variable definition. Calls clear_resize() at the end
//#define QS_DEF_RES(TYPE, name, len)                                                                                                                            \
//    static _ReusableVariablesPool<_GET_TYPE(TYPE)> _POOL_##name;                                                                          \
//    int _POOL_##name##_idx;                                                                                                                                    \
//    _GET_TYPE(TYPE)& name = _POOL_##name.getVacant(_POOL_##name##_idx);                                                                                       \
//    _ReusableVariablesAutoRelease<_GET_TYPE(TYPE)> _POOL_##name##_auto_release;                                                                                \
//    _POOL_##name##_auto_release.init(_POOL_##name##_idx, &_POOL_##name);                                                                                  \
//    name.clear_resize(len);
// Use this for debug purposes if you suspect QS_DEF in something bad
#define QS_DEF_RES(TYPE, name, len) TYPE name; name.clear_resize(len);

// Reusable class members definition
// By tradition this macros start with TL_, but should start with SL_
// To work with them you should first define commom pool with CP_DECL,
// then define it in the source class with CP_DEF(cls), and initialize
// in the constructor via CP_INIT before any TL_CP_ initializations
//

// Add this to class definition
#define TL_CP_DECL(TYPE, name)                                                                                                                                 \
    TYPE name

// Add this to constructor initialization list
#define TL_CP_GET(name) name()

#define CP_DECL                                                                                                                                                \
    bool _cp_decl_pseudo_init;

#define CP_INIT _cp_decl_pseudo_init(true)

#define CP_DEF(cls)

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __tlscont_h__
