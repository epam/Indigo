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

namespace indigo
{
    inline constexpr const char* KetConnections = "connections";
    // A haptic connection is keyed by "type", the monomer ones by "connectionType"
    // (KET-CONTRACT.md §5.1) - reading the wrong key silently skips the connection.
    inline constexpr const char* KetConnectionKind = "type";
    inline constexpr const char* KetConnectionType = "connectionType";
    inline constexpr const char* KetConnectionHaptic = "haptic";

    inline constexpr const char* KetEndpoint1 = "endpoint1";
    inline constexpr const char* KetEndpoint2 = "endpoint2";
    inline constexpr const char* KetMoleculeId = "moleculeId";
    inline constexpr const char* KetAtomId = "atomId";
    inline constexpr const char* KetAttachmentGroupId = "attachmentGroupId";

    inline constexpr const char* KetAttachmentGroups = "attachmentGroups";
    inline constexpr const char* KetGroupId = "id";
    inline constexpr const char* KetGroupAtoms = "atoms";

    inline constexpr const char* KetMoleculePrefix = "mol";
}

#endif
