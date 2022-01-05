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
        if (typeMap.find(name))                                                                                                                                \
            throw Error("Option \"%s\" already defined", name);                                                                                                \
        typeMap.insert(name, type);                                                                                                                            \
        map.insert(name, func);                                                                                                                                \
    }

#define DEF_SET_GET_OPT_HANDLERS(suffix, fSetType, fGetType, type, mapSet, mapGet)                                                                             \
    void setOptionHandler##suffix(const char* name, fSetType setFunc, fGetType getFunc)                                                                        \
    {                                                                                                                                                          \
        if (typeMap.find(name))                                                                                                                                \
            throw Error("Option \"%s\" already defined", name);                                                                                                \
        typeMap.insert(name, type);                                                                                                                            \
        mapSet.insert(name, setFunc);                                                                                                                          \
        mapGet.insert(name, getFunc);                                                                                                                          \
    }

#define CHECK_OPT_DEFINED(name)                                                                                                                                \
    if (!typeMap.find(name))                                                                                                                                   \
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
    IndigoOptionManager() = default;

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

    DECL_ERROR;
    DEF_SET_GET_OPT_HANDLERS(String, optf_string_t, get_optf_string_t, OPTION_STRING, stringSetters, stringGetters)
    DEF_SET_GET_OPT_HANDLERS(Int, optf_int_t, get_optf_int_t, OPTION_INT, intSetters, intGetters)
    DEF_SET_GET_OPT_HANDLERS(Bool, optf_bool_t, get_optf_bool_t, OPTION_BOOL, boolSetters, boolGetters)
    DEF_SET_GET_OPT_HANDLERS(Float, optf_float_t, get_optf_float_t, OPTION_FLOAT, floatSetters, floatGetters)
    DEF_SET_GET_OPT_HANDLERS(Color, optf_color_t, get_optf_color_t, OPTION_COLOR, colorSetters, colorGetters)
    DEF_SET_GET_OPT_HANDLERS(XY, optf_xy_t, get_optf_xy_t, OPTION_XY, xySetters, xyGetters)
    DEF_HANDLER(Void, optf_void_t, OPTION_VOID, voidFunctions)

    bool hasOptionHandler(const char* name) const;

    void callOptionHandlerInt(const char* name, int value);
    void callOptionHandlerBool(const char* name, int value);
    void callOptionHandlerFloat(const char* name, float value);
    void callOptionHandlerColor(const char* name, float r, float g, float b);
    void callOptionHandlerXY(const char* name, int x, int y);
    void callOptionHandlerVoid(const char* name);
    void callOptionHandler(const char* name, const char* value);

    void getOptionValueStr(const char* name, Array<char>& value) const;
    void getOptionValueInt(const char* name, int& value) const;
    void getOptionValueBool(const char* name, int& value) const;
    void getOptionValueFloat(const char* name, float& value) const;
    void getOptionValueColor(const char* name, float& r, float& g, float& b) const;
    void getOptionValueXY(const char* name, int& x, int& y) const;

    int nOptions() const;

    void getOptionType(const char* name, Array<char>& value) const;

    static _SessionLocalContainer<sf::safe_shared_hide_obj<IndigoOptionManager>>& getIndigoOptionManager();

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

    RedBlackStringMap<OPTION_TYPE, false> typeMap;

    RedBlackStringMap<optf_string_t, false> stringSetters;
    RedBlackStringMap<optf_int_t, false> intSetters;
    RedBlackStringMap<optf_bool_t, false> boolSetters;
    RedBlackStringMap<optf_float_t, false> floatSetters;
    RedBlackStringMap<optf_color_t, false> colorSetters;
    RedBlackStringMap<optf_xy_t, false> xySetters;

    RedBlackStringMap<optf_void_t, false> voidFunctions;

    RedBlackStringMap<get_optf_string_t, false> stringGetters;
    RedBlackStringMap<get_optf_int_t, false> intGetters;
    RedBlackStringMap<get_optf_bool_t, false> boolGetters;
    RedBlackStringMap<get_optf_float_t, false> floatGetters;
    RedBlackStringMap<get_optf_color_t, false> colorGetters;
    RedBlackStringMap<get_optf_xy_t, false> xyGetters;

    template <typename T>
    void callOptionHandlerT(const char* name, T arg)
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
