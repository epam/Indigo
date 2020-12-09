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

#include "molecule/json_loader.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "gzip/gzip_scanner.h"
#include <string>

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(JSONLoader, "JSON loader");

CP_DEF(JSONLoader);

JSONLoader::JSONLoader( Scanner& scanner ) : CP_INIT, _current_number( 0 ), _molecules( kArrayType ), _rgroups( kArrayType )
{
    Array<char> buf;
    scanner.readAll(buf);
    buf.push(0);
    if ( _data.Parse(buf.ptr()).HasParseError())
        throw Error("Error at parsing JSON: %s", buf.ptr());
    if( _data.HasMember( "root" ) )
    {
        const Value& root = _data["root"];
        const Value& nodes = root["nodes"];
        for( int i = 0; i < nodes.Size(); ++ i  )
        {
            const char* node_name = nodes[i]["$ref"].GetString();
            Value& node = _data[ node_name ];
            std::string type = node["type"].GetString();
            if( type.compare("molecule") == 0 )
              _molecules.PushBack( node, _data.GetAllocator());
            else if( type.compare("rgroup") == 0 )
              _rgroups.PushBack( node, _data.GetAllocator() );
            else
                throw Error("Unknow node type: %s", type.c_str());
        }
    } else
        throw Error("Error at parsing JSON. No root element: %s", buf.ptr());
}

JSONLoader::~JSONLoader()
{
}

bool JSONLoader::hasNext()
{
    return _current_number < count();
}

const Value& JSONLoader::next()
{
    return _molecules[_current_number++];
}

const Value& JSONLoader::at( int index )
{
    return _molecules[index];
}

int JSONLoader::currentNumber()
{
    return _current_number;
}

int JSONLoader::count()
{
    return _molecules.Size();
}

const Value& JSONLoader::rgroups()
{
    return _rgroups;
}


