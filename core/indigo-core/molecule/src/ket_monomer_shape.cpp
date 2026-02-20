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

#include "molecule/ket_monomer_shape.h"
#include <map>

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;

IMPL_ERROR(KetMonomerShape, "Monomer Shape")

KetMonomerShape::KetMonomerShape(const std::string& id, bool collapsed, const std::string& shape, Vec2f position, const std::vector<std::string>& monomers)
    : KetObjWithProps(), _id(id), _collapsed(collapsed), _shape(strToShapeType(shape)), _position(position), _monomers(monomers)
{
}

KetMonomerShape::shape_type KetMonomerShape::strToShapeType(std::string shape)
{
    static std::map<std::string, KetMonomerShape::shape_type> str_to_shape{
        {"generic", shape_type::generic},
        {"antibody", shape_type::antibody},
        {"double helix", shape_type::double_helix},
        {"globular protein", shape_type::globular_protein},
    };
    auto it = str_to_shape.find(shape);
    if (it == str_to_shape.end())
        throw Error("Unknown shape type %s", shape.c_str());
    return it->second;
}

std::string KetMonomerShape::shapeTypeToStr(shape_type shape)
{
    static std::map<KetMonomerShape::shape_type, std::string> shape_to_str{
        {shape_type::generic, "generic"},
        {shape_type::antibody, "antibody"},
        {shape_type::double_helix, "double helix"},
        {shape_type::globular_protein, "globular protein"},
    };
    auto it = shape_to_str.find(shape);
    if (it == shape_to_str.end())
        throw Error("Unknown shape type %d", shape);
    return it->second;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
