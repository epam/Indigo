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

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

#include "molecule/ket_commons.h"
#include "base_cpp/scanner.h"
#include "molecule/monomer_commons.h"
#include <cppcodec/base64_default_rfc4648.hpp>

namespace indigo
{
    uint8_t getPointSide(const Vec2f& point, const Vec2f& beg, const Vec2f& end)
    {
        uint8_t bit_mask = 0;
        Vec2f arrow_vec(beg);
        arrow_vec.sub(end);

        Vec2f slope1(point.x, point.y);
        Vec2f slope2(slope1);
        slope1.sub(beg);
        slope2.sub(end);
        auto dt1 = Vec2f::dot(slope1, arrow_vec);
        auto dt2 = Vec2f::dot(slope2, arrow_vec);

        if (std::signbit(dt1))
            bit_mask |= KETReagentUpArea;

        if (std::signbit(dt2))
            bit_mask |= KETReagentDownArea;

        return bit_mask;
    }

    bool isCIPSGroup(SGroup& sgroup)
    {
        if (sgroup.sgroup_type == SGroup::SG_DATA)
        {
            auto& dsg = (DataSGroup&)sgroup;
            return std::string(dsg.name.ptr()) == "INDIGO_CIP_DESC";
        }
        return false;
    }

    CIPDesc stringToCIP(const std::string& cip_str)
    {
        static const std::unordered_map<std::string, CIPDesc> KStringToCIP = {{"R", CIPDesc::R}, {"S", CIPDesc::S}, {"r", CIPDesc::r},
                                                                              {"s", CIPDesc::s}, {"E", CIPDesc::E}, {"Z", CIPDesc::Z}};
        auto cip_it = KStringToCIP.find(cip_str);
        if (cip_it != KStringToCIP.end())
            return cip_it->second;
        return CIPDesc::NONE;
    }

    std::string CIPToString(CIPDesc cip)
    {
        static const std::unordered_map<int, std::string> KCIPToString = {{(int)CIPDesc::R, "R"}, {(int)CIPDesc::S, "S"}, {(int)CIPDesc::r, "r"},
                                                                          {(int)CIPDesc::s, "s"}, {(int)CIPDesc::E, "E"}, {(int)CIPDesc::Z, "Z"}};
        auto cip_it = KCIPToString.find((int)cip);
        std::string res;
        if (cip_it != KCIPToString.end())
            res = cip_it->second;
        return res;
    }

    void getSGroupAtoms(BaseMolecule& mol, std::list<std::unordered_set<int>>& neighbors)
    {
        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sgroup = mol.sgroups.getSGroup(i);
            neighbors.push_back({});
            auto& sg_set = neighbors.back();
            for (auto atom_idx : sgroup.atoms)
                sg_set.insert(atom_idx);
        }
        if (mol.isQueryMolecule())
        {
            QueryMolecule& qmol = static_cast<QueryMolecule&>(mol);
            qmol.getComponentNeighbors(neighbors);
        }
    }

    std::string convertAPToHELM(const std::string& atp_id_str)
    {
        if (::isupper(atp_id_str[0]) && atp_id_str.size() == 2)
        {
            if (atp_id_str == kLeftAttachmentPoint)
                return kAttachmentPointR1;
            else if (atp_id_str == kRightAttachmentPoint)
                return kAttachmentPointR2;
            else if (atp_id_str[1] == 'x')
                return std::string("R") + std::to_string(atp_id_str[0] - 'A' + 1);
        }
        return atp_id_str;
    }

    std::string convertAPFromHELM(const std::string& atp_id_str)
    {
        int id = extract_id(atp_id_str, "R");
        if (id < 0)
            throw std::invalid_argument(std::string("convertAPFromHELM: prefix 'R' not found in :") + atp_id_str);
        char ap_symbol = static_cast<char>(id) + '@'; // convert number to ASCII letter
        std::string res(1, ap_symbol);
        switch (ap_symbol)
        {
        case 'A':
            res += 'l';
            break;
        case 'B':
            res += 'r';
            break;
        default:
            res += 'x';
            break;
        }
        return res;
    }

    KETImage::KETImage(const Rect2f& bbox, KETImage::ImageFormat format, const std::string& base64) : MetaObject(CID), _bbox(bbox), _image_format(format)
    {
        BufferScanner b64decode(base64.c_str(), true);
        b64decode.readAll(_image_data);
    }

    std::string KETImage::getBase64() const
    {
        return base64::encode(_image_data.c_str(), _image_data.size());
    }

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
