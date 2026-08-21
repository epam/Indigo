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

// Field names of the KET wire format, shared by its readers and writers.
// Covers the haptic bond feature (#3233) only; the rest of the vocabulary is
// still inline. Contract: Task/3233/KET-CONTRACT.md.
//
// A `Field` suffix marks a JSON key. Names without it are KET *values*, in the
// style of KetConnectionSingle / KetConnectionHydro (monomers_defs.h). Neither
// kind is a type, though Ket* is also the prefix of the classes in ket_objects.h.

namespace indigo
{
    // Generic keys: each is spelled the same wherever it appears, but says nothing
    // about what the object around it is. "type" alone distinguishes a molecule
    // node from an rgroup, a monomerTemplate, a query component and an atom list.
    inline constexpr const char* KetTypeField = "type";
    inline constexpr const char* KetIdField = "id";
    inline constexpr const char* KetAtomsField = "atoms";

    // Connections of the root.
    inline constexpr const char* KetConnectionsField = "connections";
    // A haptic connection is discriminated by KetTypeField, the monomer ones carry
    // this key instead (KET-CONTRACT.md §5.1). Reading the wrong one silently skips
    // the connection.
    inline constexpr const char* KetConnectionTypeField = "connectionType";
    inline constexpr const char* KetConnectionHaptic = "haptic"; // value of KetTypeField

    inline constexpr const char* KetEndpoint1Field = "endpoint1";
    inline constexpr const char* KetEndpoint2Field = "endpoint2";
    inline constexpr const char* KetMoleculeIdField = "moleculeId";
    inline constexpr const char* KetAtomIdField = "atomId";
    inline constexpr const char* KetAttachmentGroupIdField = "attachmentGroupId";

    // Attachment groups, declared by a molecule node; a group object uses the
    // generic KetIdField and KetAtomsField above.
    inline constexpr const char* KetAttachmentGroupsField = "attachmentGroups";

    // Not a key: the prefix a molecule node is referenced by, as in "mol0".
    inline constexpr const char* KetMoleculeRefPrefix = "mol";
}

#endif
