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

#include "molecule/transformation.h"

#include <map>

using namespace indigo;

IMPL_ERROR(Transformation, "Transformation");

static const std::map<std::string, Transformation::FlipType> flip_types = {
    {"", Transformation::FlipType::none}, {"horizontal", Transformation::FlipType::horizontal}, {"vertical", Transformation::FlipType::vertical}};

Transformation::Transformation(float rotation, const Vec2f& shift, std::string flip_type) : rotate(rotation), shift(shift)
{
    auto it = flip_types.find(flip_type);
    if (it != flip_types.end())
        flip = it->second;
    else
        throw Error("Invalid flip value: %s", flip_type.c_str());
}

const std::string Transformation::getFlip() const
{
    for (auto it : flip_types)
    {
        if (it.second == flip)
            return it.first;
    }
    return "";
}