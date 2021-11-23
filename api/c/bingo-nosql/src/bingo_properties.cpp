#include "bingo_properties.h"

#include "base_cpp/exception.h"
#include "base_cpp/profiling.h"

#include <algorithm>
#include <fstream>
#include <limits.h>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace bingo;
using namespace indigo;

Properties::Properties()
{
}

MMFAddress Properties::create(MMFPtr<Properties>& ptr)
{
    ptr.allocate();
    new (ptr.ptr()) Properties();
    return ptr.getAddress();
}

void Properties::load(MMFPtr<Properties>& ptr, MMFAddress offset)
{
    ptr = MMFPtr<Properties>(offset);
}

void Properties::parseOptions(const char* options, std::map<std::string, std::string>& option_map, std::vector<std::string>* allowed_props)
{
    if (options == 0 || strlen(options) == 0)
        return;

    option_map.clear();

    std::stringstream options_stream;
    options_stream << options;

    std::string line;
    while (options_stream.good())
    {
        std::getline(options_stream, line, ';');

        if (line.size() == 0)
            continue;

        std::string opt_name, opt_value;
        int sep = (int)line.find_first_of(':');

        if (sep != -1)
            opt_name.assign(line.substr(0, sep));

        opt_value.assign(line.substr(sep + 1, std::string::npos));

        if (allowed_props)
        {
            if (std::find(allowed_props->begin(), allowed_props->end(), opt_name) == allowed_props->end())
                throw Exception("Properties: Incorrect parameters");
        }

        option_map.insert(std::pair<std::string, std::string>(opt_name, opt_value));
    }
}

void Properties::add(const char* prop_name, const char* value)
{
    int prop_id;

    for (prop_id = 0; prop_id < _props.size(); prop_id++)
        if (strcmp(_props[prop_id].name.ptr(), prop_name) == 0)
            break;

    if (prop_id == _props.size())
    {
        _PropertyPair& new_pair = _props.push();
        new_pair.name.allocate(strlen(prop_name) + 1);
        strcpy(new_pair.name.ptr(), prop_name);

        new_pair.value.allocate(max_prop_len);
    }

    if (strlen(value) >= max_prop_len)
        throw Exception("BingoProperties: Too long property value");

    strcpy(_props[prop_id].value.ptr(), value);
}

void Properties::add(const char* prop_name, unsigned long value)
{
    std::ostringstream osstr;
    osstr << value;

    add(prop_name, osstr.str().c_str());
}

const char* Properties::getNoThrow(const char* prop_name) const
{
    int prop_id;

    for (prop_id = 0; prop_id < _props.size(); prop_id++)
        if (strcmp(_props[prop_id].name.ptr(), prop_name) == 0)
            break;

    if (prop_id == _props.size())
        return 0;

    return _props[prop_id].value.ptr();
}

const char* Properties::get(const char* prop_name)
{
    const char* res = getNoThrow(prop_name);

    if (res == nullptr)
    {
        throw Exception("Unknown property field");
    }
    return res;
}

unsigned long Properties::getULongNoThrow(const char* prop_name)
{
    const char* value = getNoThrow(prop_name);

    if (value == 0)
        return ULONG_MAX;

    unsigned long u_dec;
    std::istringstream isstr(value);
    isstr >> u_dec;

    return u_dec;
}

unsigned long Properties::getULong(const char* prop_name)
{
    unsigned long res = getULongNoThrow(prop_name);

    if (res == ULONG_MAX)
        throw Exception("Unknown property field");

    return res;
}

void Properties::_parseProperty(const std::string& line, std::string& prop_out, std::string& value_out)
{
    int sep = (int)line.find_first_of('=');

    prop_out.assign(line.substr(0, sep));
    value_out.assign(line.substr(sep + 1, std::string::npos));
}
