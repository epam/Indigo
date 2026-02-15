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

#ifndef __ket_monomer_shape__
#define __ket_monomer_shape__

#include "common/base_c/defs.h"
#include "common/base_cpp/exception.h"
#include "common/math/algebra.h"
#include "molecule/ket_obj_with_props.h"
#include <vector>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT KetMonomerShape : public KetObjWithProps
    {
    public:
        DECL_ERROR;

        inline static std::string ref_prefix = "monomerShape-";

        enum class shape_type
        {
            generic,
            antibody,
            double_helix,
            globular_protein
        };

        KetMonomerShape(const std::string& id, bool collapsed, const std::string& shape, Vec2f position, const std::vector<std::string>& monomers);

        const std::string& id() const
        {
            return _id;
        }

        bool collapsed() const
        {
            return _collapsed;
        }

        shape_type shape() const
        {
            return _shape;
        }

        static shape_type strToShapeType(std::string shape);

        static std::string shapeTypeToStr(shape_type shape);

        Vec2f position() const
        {
            return _position;
        }

        const std::vector<std::string>& monomers() const
        {
            return _monomers;
        }

    private:
        std::string _id;
        bool _collapsed;
        shape_type _shape;
        Vec2f _position;
        std::vector<std::string> _monomers;
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
