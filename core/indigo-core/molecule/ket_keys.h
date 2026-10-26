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

#ifndef __ket_keys__
#define __ket_keys__

#include "molecule/molecule_sgroups.h"

// Field names of the KET wire format, shared by its readers and writers.
// Covers haptic bonds and S-groups; the rest of the vocabulary is still inline.
//
// A KetKey prefix marks a JSON key. Names without it are KET *values*, in the style
// of KetConnectionSingle / KetConnectionHydro (monomers_defs.h). The prefix also
// keeps both apart from the Ket* classes of ket_objects.h - KetKeyEndpoint1 cannot
// be mistaken for KetConnectionEndPoint.

namespace indigo
{
    // Generic keys: each is spelled the same wherever it appears, but says nothing
    // about what the object around it is. "type" alone distinguishes a molecule
    // node from an rgroup, a monomerTemplate, a query component and an atom list.
    inline constexpr const char* KetKeyType = "type";
    inline constexpr const char* KetKeyId = "id";
    inline constexpr const char* KetKeyAtoms = "atoms";

    // Connections of the root.
    inline constexpr const char* KetKeyConnections = "connections";
    // A haptic connection is discriminated by KetKeyType, the monomer ones carry
    // this key instead (KET-CONTRACT.md §5.1). Reading the wrong one silently skips
    // the connection.
    inline constexpr const char* KetKeyConnectionType = "connectionType";
    inline constexpr const char* KetConnectionHaptic = "haptic"; // value of KetKeyType

    inline constexpr const char* KetKeyEndpoint1 = "endpoint1";
    inline constexpr const char* KetKeyEndpoint2 = "endpoint2";
    inline constexpr const char* KetKeyMoleculeId = "moleculeId";
    inline constexpr const char* KetKeyAtomId = "atomId";
    inline constexpr const char* KetKeyAttachmentGroupId = "attachmentGroupId";

    // Attachment groups, declared by a molecule node; a group object uses the
    // generic KetKeyId and KetKeyAtoms above.
    inline constexpr const char* KetKeyAttachmentGroups = "attachmentGroups";

    // On an atom and on an attachment group alike; a group omits a value of 0.
    inline constexpr const char* KetKeyCharge = "charge";
    inline constexpr const char* KetKeyRadical = "radical";

    // S-groups of a molecule node. An S-group uses the generic KetKeyType, KetKeyId and
    // KetKeyAtoms above; its type is a name of the S-group type table, spelled exactly.
    inline constexpr const char* KetKeySGroups = "sgroups";
    inline constexpr const char* KetKeyBonds = "bonds";
    inline constexpr const char* KetKeyParentId = "parentId";
    inline constexpr const char* KetKeyConnectivity = "connectivity";
    inline constexpr const char* KetKeySubtype = "subtype";
    inline constexpr const char* KetKeyComponentOrder = "componentOrder";
    inline constexpr const char* KetKeyMultiplier = "mul";
    inline constexpr const char* KetSGroupQueryComponent = "queryComponent"; // value of KetKeyType, a query feature

    // The S-group types KET carries; the others are refused by its reader and its writer.
    inline bool isKetSGroupType(int sg_type)
    {
        switch (sg_type)
        {
        case SGroup::SG_TYPE_GEN:
        case SGroup::SG_TYPE_MUL:
        case SGroup::SG_TYPE_SRU:
        case SGroup::SG_TYPE_SUP:
        case SGroup::SG_TYPE_DAT:
        case SGroup::SG_TYPE_COP:
        case SGroup::SG_TYPE_COM:
        case SGroup::SG_TYPE_MIX:
        case SGroup::SG_TYPE_FOR:
            return true;
        default:
            return false;
        }
    }

    // The fragments of an R-group node.
    inline constexpr const char* KetKeyFragments = "fragments";

    // Not a key: the prefix a molecule node is referenced by, as in "mol0".
    inline constexpr const char* KetMoleculeRefPrefix = "mol";
}

#endif
