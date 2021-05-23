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

#ifndef __otion_manager_h__
#define __otion_manager_h__

#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/red_black.h"

#include <sstream>

using namespace indigo;

#define DECL_SET_OPT_HANDLER(suffix, ftype, type, map) DLLEXPORT void setOptionHandler##suffix(const char* name, ftype func);

#define DEF_HANDLER(suffix, ftype, type, map)                                                                                                                  \
    void setOptionHandler##suffix(const char* name, ftype func)                                                                                                \
    {                                                                                                                                                          \
        if (typeMap.find(name) != typeMap.end())                                                                                                                                \
            throw Error("Option \"%s\" already defined", name);                                                                                                \
        typeMap.emplace(name, type);                                                                                                                            \
        map.emplace(name, func);                                                                                                                                \
    }

#define DEF_SET_GET_OPT_HANDLERS(suffix, fSetType, fGetType, type, mapSet, mapGet)                                                                             \
    void setOptionHandler##suffix(const char* name, fSetType setFunc, fGetType getFunc)                                                                        \
    {                                                                                                                                                          \
        if (typeMap.find(name) != typeMap.end())                                                                                                                                \
            throw Error("Option \"%s\" already defined", name);                                                                                                \
        typeMap.emplace(name, type);                                                                                                                            \
        mapSet.emplace(name, setFunc);                                                                                                                          \
        mapGet.emplace(name, getFunc);                                                                                                                          \
    }

#define CHECK_OPT_DEFINED(name)                                                                                                                                \
    if ( typeMap.find(name) == typeMap.end() )                                                                                                                                   \
    throw Error("Property \"%s\" not defined", name)

#define CHECK_OPT_TYPE(name, type)                                                                                                                             \
    if (typeMap.at(name) != type)                                                                                                                              \
    throw Error("Property type mismatch", name)

#define SETTER_GETTER_BOOL_OPTION(option) [](int enabled) { option = (enabled != 0); }, [](int& enabled) { enabled = (option != 0); }

#define SETTER_GETTER_INT_OPTION(option) [](int value) { option = value; }, [](int& value) { value = option; }

#define SETTER_GETTER_FLOAT_OPTION(option) [](float value) { option = value; }, [](float& value) { value = option; }

#define SETTER_GETTER_COLOR_OPTION(option)                                                                                                                     \
    [](float r, float g, float b) { option.set(r, g, b); }, [](float& r, float& g, float& b) {                                                                 \
        r = option.x;                                                                                                                                          \
        g = option.y;                                                                                                                                          \
        b = option.z;                                                                                                                                          \
    }

#define SETTER_GETTER_XY_OPTION(optionX, optionY)                                                                                                              \
    [](int x, int y) {                                                                                                                                         \
        optionX = x;                                                                                                                                           \
        optionY = y;                                                                                                                                           \
    },                                                                                                                                                         \
        [](int& x, int& y) {                                                                                                                                   \
            x = optionX;                                                                                                                                       \
            y = optionY;                                                                                                                                       \
        }

#define SETTER_GETTER_STR_OPTION(option)                                                                                                                       \
    [](const char* value) { option.readString(value, true); }, [](Array<char>& value) {                                                                        \
        value.copy(option);                                                                                                                                    \
        value.push(0);                                                                                                                                         \
    }

class DLLEXPORT IndigoOptionManager
{
public:
    typedef void (*optf_string_t)(const char*);
    typedef void (*optf_int_t)(int);
    typedef void (*optf_bool_t)(int);
    typedef void (*optf_float_t)(float);
    typedef void (*optf_color_t)(float, float, float);
    typedef void (*optf_xy_t)(int, int);
    typedef void (*optf_void_t)();

    typedef void (*get_optf_string_t)(Array<char>& value);
    typedef void (*get_optf_int_t)(int&);
    typedef void (*get_optf_bool_t)(int&);
    typedef void (*get_optf_float_t)(float&);
    typedef void (*get_optf_color_t)(float&, float&, float&);
    typedef void (*get_optf_xy_t)(int&, int&);

    IndigoOptionManager();

    DECL_ERROR;
    DEF_SET_GET_OPT_HANDLERS(String, optf_string_t, get_optf_string_t, OPTION_STRING, stringSetters, stringGetters)
    DEF_SET_GET_OPT_HANDLERS(Int, optf_int_t, get_optf_int_t, OPTION_INT, intSetters, intGetters)
    DEF_SET_GET_OPT_HANDLERS(Bool, optf_bool_t, get_optf_bool_t, OPTION_BOOL, boolSetters, boolGetters)
    DEF_SET_GET_OPT_HANDLERS(Float, optf_float_t, get_optf_float_t, OPTION_FLOAT, floatSetters, floatGetters)
    DEF_SET_GET_OPT_HANDLERS(Color, optf_color_t, get_optf_color_t, OPTION_COLOR, colorSetters, colorGetters)
    DEF_SET_GET_OPT_HANDLERS(XY, optf_xy_t, get_optf_xy_t, OPTION_XY, xySetters, xyGetters)
    DEF_HANDLER(Void, optf_void_t, OPTION_VOID, voidFunctions)

    bool hasOptionHandler(const char* name);

    void callOptionHandlerInt(const char* name, int value);
    void callOptionHandlerBool(const char* name, int value);
    void callOptionHandlerFloat(const char* name, float value);
    void callOptionHandlerColor(const char* name, float r, float g, float b);
    void callOptionHandlerXY(const char* name, int x, int y);
    void callOptionHandlerVoid(const char* name);
    void callOptionHandler(const char* name, const char* value);

    void getOptionValueStr(const char* name, Array<char>& value);
    void getOptionValueInt(const char* name, int& value);
    void getOptionValueBool(const char* name, int& value);
    void getOptionValueFloat(const char* name, float& value);
    void getOptionValueColor(const char* name, float& r, float& g, float& b);
    void getOptionValueXY(const char* name, int& x, int& y);

    int nOptions() const;

    void getOptionType(const char* name, Array<char>& value);

    OsLock lock;

protected:
    enum OPTION_TYPE
    {
        OPTION_STRING,
        OPTION_INT,
        OPTION_BOOL,
        OPTION_FLOAT,
        OPTION_COLOR,
        OPTION_XY,
        OPTION_VOID
    };

    int _parseInt(const char* str, int& val);
    int _parseBool(const char* str, int& val);
    int _parseFloat(const char* str, float& val);
    int _parseColor(const char* str, float& r, float& g, float& b);
    int _parseSize(const char* str, int& w, int& h);

    StringMapNC<OPTION_TYPE> typeMap;

    StringMapNC<optf_string_t> stringSetters;
    StringMapNC<optf_int_t> intSetters;
    StringMapNC<optf_bool_t> boolSetters;
    StringMapNC<optf_float_t> floatSetters;
    StringMapNC<optf_color_t> colorSetters;
    StringMapNC<optf_xy_t> xySetters;

    StringMapNC<optf_void_t> voidFunctions;

    StringMapNC<get_optf_string_t> stringGetters;
    StringMapNC<get_optf_int_t> intGetters;
    StringMapNC<get_optf_bool_t> boolGetters;
    StringMapNC<get_optf_float_t> floatGetters;
    StringMapNC<get_optf_color_t> colorGetters;
    StringMapNC<get_optf_xy_t> xyGetters;

    template <typename T> void callOptionHandlerT(const char* name, T arg)
    {
        // Convert to string for default string parsing
        std::stringstream ss;
        ss << arg;
        std::string converted = ss.str();
        callOptionHandler(name, converted.c_str());
    }

private:
    IndigoOptionManager(const IndigoOptionManager&);
};

#endif //__otion_manager_h__
