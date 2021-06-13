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
#include <string>

#include "base_cpp/scanner.h"
#include "option_manager.h"

ThreadSafeStaticObj<IndigoOptionManager> indigo_option_manager;

DLLEXPORT IndigoOptionManager& indigoGetOptionManager()
{
    return indigo_option_manager.ref();
}

IMPL_ERROR(IndigoOptionManager, "option manager");

IndigoOptionManager::IndigoOptionManager()
{
}

void IndigoOptionManager::callOptionHandlerInt(const char* name, int value)
{
    CHECK_OPT_DEFINED(name);

    if (typeMap.at(name) == OPTION_BOOL && (value == 0 || value == 1))
    {
        boolSetters.at(name)(value);
        return;
    }

    if (typeMap.at(name) == OPTION_INT)
        intSetters.at(name)(value);
    else
        callOptionHandlerT(name, value);
}

void IndigoOptionManager::callOptionHandlerBool(const char* name, int value)
{
    CHECK_OPT_DEFINED(name);
    if (typeMap.at(name) == OPTION_BOOL)
        boolSetters.at(name)(value);
    else
        callOptionHandlerT(name, value);
}

void IndigoOptionManager::callOptionHandlerFloat(const char* name, float value)
{
    CHECK_OPT_DEFINED(name);
    if (typeMap.at(name) == OPTION_FLOAT)
        floatSetters.at(name)(value);
    else
        callOptionHandlerT(name, value);
}

void IndigoOptionManager::callOptionHandlerVoid(const char* name)
{
    CHECK_OPT_DEFINED(name);
    voidFunctions.at(name)();
}

void IndigoOptionManager::callOptionHandlerColor(const char* name, float r, float g, float b)
{
    CHECK_OPT_DEFINED(name);
    CHECK_OPT_TYPE(name, OPTION_COLOR);
    colorSetters.at(name)(r, g, b);
}

void IndigoOptionManager::callOptionHandlerXY(const char* name, int x, int y)
{
    CHECK_OPT_DEFINED(name);
    CHECK_OPT_TYPE(name, OPTION_XY);
    xySetters.at(name)(x, y);
}

void IndigoOptionManager::getOptionValueInt(const char* name, int& value)
{
    CHECK_OPT_DEFINED(name);
    CHECK_OPT_TYPE(name, OPTION_INT);
    intGetters.at(name)(value);
}

void IndigoOptionManager::getOptionValueStr(const char* name, ArrayChar& value)
{
    CHECK_OPT_DEFINED(name);

    switch (typeMap.at(name))
    {
    case OPTION_STRING: {
        CHECK_OPT_TYPE(name, OPTION_STRING);
        stringGetters.at(name)(value);
        break;
    }
    case OPTION_INT: {
        int tmp;
        getOptionValueInt(name, tmp);
        auto strValue = std::to_string(tmp);
        value.readString(strValue.c_str(), true);
        break;
    }
    case OPTION_BOOL: {
        int tmp;
        getOptionValueBool(name, tmp);
        std::string strValue = "false";
        if (tmp == 1)
            strValue = "true";
        value.readString(strValue.c_str(), true);
        break;
    }
    case OPTION_FLOAT: {
        float tmp;
        getOptionValueFloat(name, tmp);
        std::stringstream strValue;
        strValue << tmp;
        value.readString(strValue.str().c_str(), true);
        break;
    }
    case OPTION_COLOR: {
        float r, g, b;
        getOptionValueColor(name, r, g, b);
        std::stringstream coords;

        coords << "[" << r << ", " << g << ", " << b << "]";
        value.readString(coords.str().c_str(), true);
        break;
    }
    case OPTION_XY: {
        int x, y;
        getOptionValueXY(name, x, y);
        std::stringstream coords;

        coords << "[" << x << ", " << y << "]";
        value.readString(coords.str().c_str(), true);
        break;
    }
    default:
        throw Error("Property type mismatch", name);
        break;
    }
}

void IndigoOptionManager::getOptionValueBool(const char* name, int& value)
{
    CHECK_OPT_DEFINED(name);
    CHECK_OPT_TYPE(name, OPTION_BOOL);
    boolGetters.at(name)(value);
}

void IndigoOptionManager::getOptionValueFloat(const char* name, float& value)
{
    CHECK_OPT_DEFINED(name);
    CHECK_OPT_TYPE(name, OPTION_FLOAT);
    floatGetters.at(name)(value);
}

void IndigoOptionManager::getOptionValueColor(const char* name, float& r, float& g, float& b)
{
    CHECK_OPT_DEFINED(name);
    CHECK_OPT_TYPE(name, OPTION_COLOR);
    colorGetters.at(name)(r, g, b);
}

void IndigoOptionManager::getOptionValueXY(const char* name, int& x, int& y)
{
    CHECK_OPT_DEFINED(name);
    CHECK_OPT_TYPE(name, OPTION_XY);
    xyGetters.at(name)(x, y);
}

void IndigoOptionManager::getOptionType(const char* name, ArrayChar& value)
{
    CHECK_OPT_DEFINED(name);
    if (!typeMap.find(name))
        throw Error("Property \"%s\" not defined", name);

    auto copyString = [](const char* source, char* dest, int len) {
      if (strlen(source) > len)
          throw Error("invalid string value len: expected len: %d, actual len: %d", len, strlen(source));
      memset(dest, ' ', len);
      strcpy(dest, source);
    };
    switch (typeMap.at(name))
    {
    case OPTION_STRING:
        value.readString("str", true);
        break;
    case OPTION_INT:
        value.readString("int", true);
        break;
    case OPTION_BOOL:
        value.readString("bool", true);
        break;
    case OPTION_FLOAT:
        value.readString("float", true);
        break;
    case OPTION_COLOR:
        value.readString("color", true);
        break;
    case OPTION_XY:
        value.readString("xy", true);
        break;
    }
}

bool IndigoOptionManager::hasOptionHandler(const char* name)
{
    return typeMap.find(name);
}

void IndigoOptionManager::callOptionHandler(const char* name, const char* value)
{
    if (!typeMap.find(name))
        throw Error("Property \"%s\" not defined", name);
    OPTION_TYPE type = typeMap.at(name);
    int x = 0, y = 0;
    float f = 0, r = 0, g = 0, b = 0;
    switch (type)
    {
    case OPTION_STRING:
        stringSetters.at(name)(value);
        break;
    case OPTION_INT:
        if (_parseInt(value, x) < 0)
            throw Error("Cannot recognize \"%s\" as an integer value", value);
        intSetters.at(name)(x);
        break;
    case OPTION_BOOL:
        if (_parseBool(value, x) < 0)
            throw Error("Cannot recognize \"%s\" as a boolean value", value);
        boolSetters.at(name)(x);
        break;
    case OPTION_FLOAT:
        if (_parseFloat(value, f) < 0)
            throw Error("Cannot recognize \"%s\" as a float value", value);
        floatSetters.at(name)(f);
        break;
    case OPTION_COLOR:
        if (_parseColor(value, r, g, b) < 0)
            throw Error("Cannot recognize \"%s\" as a color value", value);
        colorSetters.at(name)(r, g, b);
        break;
    case OPTION_XY:
        if (_parseSize(value, x, y) < 0)
            throw Error("Cannot recognize \"%s\" as a pair of integers", value);
        xySetters.at(name)(x, y);
        break;
    default:
        throw Error("Option type not supported");
    }
}

int IndigoOptionManager::nOptions() const
{
    return typeMap.size();
}

int IndigoOptionManager::_parseInt(const char* str, int& val)
{
    if (sscanf(str, "%d", &val) != 1)
        return -1;
    return 1;
}

int IndigoOptionManager::_parseBool(const char* str, int& val)
{
    if (strcasecmp(str, "true") == 0 || strcasecmp(str, "on") == 0 || strcasecmp(str, "yes") == 0)
    {
        val = 1;
        return 1;
    }
    else if (strcasecmp(str, "false") == 0 || strcasecmp(str, "off") == 0 || strcasecmp(str, "no") == 0)
    {
        val = 0;
        return 1;
    }
    else
        return _parseInt(str, val);
}

int IndigoOptionManager::_parseFloat(const char* str, float& val)
{
    BufferScanner scanner(str);
    if (!scanner.tryReadFloat(val))
        return -1;
    return 1;
}

int IndigoOptionManager::_parseColor(const char* str, float& r, float& g, float& b)
{
    BufferScanner scanner(str);
    if (!scanner.tryReadFloat(r))
        return -1;
    scanner.skipSpace();
    if (scanner.isEOF())
        return -1;
    if (scanner.readChar() != ',')
        return -1;
    scanner.skipSpace();
    if (!scanner.tryReadFloat(g))
        return -1;
    scanner.skipSpace();
    if (scanner.isEOF())
        return -1;
    if (scanner.readChar() != ',')
        return -1;
    scanner.skipSpace();
    if (!scanner.tryReadFloat(b))
        return -1;
    return 1;
}

int IndigoOptionManager::_parseSize(const char* str, int& w, int& h)
{
    if (sscanf(str, "%d,%d", &w, &h) != 2)
        return -1;
    return 1;
}
