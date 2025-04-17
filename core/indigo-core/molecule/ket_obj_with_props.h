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

#ifndef __ket_obj_with_props__
#define __ket_obj_with_props__

#include <rapidjson/document.h>

#include "common/base_c/defs.h"
#include "common/base_cpp/exception.h"

#include <map>
#include <string>
#include <type_traits>
#include <utility>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class JsonWriter;

    template <typename T>
    constexpr auto toUType(T enumerator) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(enumerator);
    }

    class DLLEXPORT KetObjWithProps
    {
    public:
        DECL_ERROR;

        virtual ~KetObjWithProps() = default;

        inline void setBoolProp(int idx, bool value)
        {
            _bool_props[idx] = value;
        };

        inline void setIntProp(int idx, int value)
        {
            _int_props[idx] = value;
        };

        inline void setIntProp(int idx, std::size_t value)
        {
            _int_props[idx] = static_cast<int>(value);
        };

        inline void setStringProp(int idx, std::string value)
        {
            _string_props[idx] = value;
        };

        void setBoolProp(std::string name, bool value);
        void setIntProp(std::string name, int value);
        void setStringProp(std::string name, std::string value);

        virtual const std::map<std::string, int>& getBoolPropStrToIdx() const;
        virtual const std::map<std::string, int>& getIntPropStrToIdx() const;
        virtual const std::map<std::string, int>& getStringPropStrToIdx() const;

        inline bool hasBoolProp(int idx) const
        {
            return _bool_props.count(idx) > 0;
        };
        inline bool hasIntProp(int idx) const
        {
            return _int_props.count(idx) > 0;
        };
        inline bool hasStringProp(int idx) const
        {
            return _string_props.count(idx) > 0;
        };

        bool getBoolProp(int idx) const;
        int getIntProp(int idx) const;
        const std::string& getStringProp(int idx) const;

        std::pair<bool, int> getBoolPropIdx(const std::string& name) const;
        std::pair<bool, int> getIntPropIdx(const std::string& name) const;
        std::pair<bool, int> getStringPropIdx(const std::string& name) const;

        bool hasBoolProp(const std::string& name) const
        {
            auto res = getBoolPropIdx(name);
            if (res.first)
                return hasBoolProp(res.second);
            return false;
        }
        bool hasIntProp(const std::string& name) const
        {
            auto res = getIntPropIdx(name);
            if (res.first)
                return hasIntProp(res.second);
            return false;
        };
        bool hasStringProp(const std::string& name) const
        {
            auto res = getStringPropIdx(name);
            if (res.first)
                return hasStringProp(res.second);
            return false;
        };

        bool getBoolProp(const std::string& name) const;
        int getIntProp(const std::string& name) const;
        const std::string& getStringProp(const std::string& name) const;

        void parseOptsFromKet(const rapidjson::Value& json);
        void saveOptsToKet(JsonWriter& writer) const;

        void copy(const KetObjWithProps& other)
        {
            _bool_props = other._bool_props;
            _int_props = other._int_props;
            _string_props = other._string_props;
        }

    private:
        std::map<int, bool> _bool_props;
        std::map<int, int> _int_props;
        std::map<int, std::string> _string_props;
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif