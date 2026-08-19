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

// Names of the KET wire format, in one place so that a reader and a writer of the
// same field cannot spell it differently. That is not a hypothetical: the highlight
// round-trip is broken in exactly that way today - the saver writes "entityType"
// values "atom"/"bond" while the loader compares against "atoms"/"bonds", so
// highlights written by Indigo are not read back by Indigo.
//
// Only the names the haptic bond feature (#3233) touches live here so far; the rest
// of the KET vocabulary is still spelled inline at ~380 distinct literals across the
// four JSON files and is a migration of its own.
//
// const char* rather than the const std::string of monomers_defs.h: these are handed
// to rapidjson HasMember()/operator[] and to JsonWriter::Key(), all of which take a
// const char*, so a std::string would only add a construction per use.

namespace indigo
{
    namespace ket
    {
        // Root-level connection array and the two keys that discriminate its members.
        // A haptic connection is keyed by "type"; the monomer ones by "connectionType".
        inline constexpr const char* KetConnections = "connections";
        inline constexpr const char* KetConnectionKind = "type";
        inline constexpr const char* KetConnectionType = "connectionType";
        inline constexpr const char* KetConnectionHaptic = "haptic";

        // Endpoints of a connection, and the ways one addresses its target.
        inline constexpr const char* KetEndpoint1 = "endpoint1";
        inline constexpr const char* KetEndpoint2 = "endpoint2";
        inline constexpr const char* KetMoleculeId = "moleculeId";
        inline constexpr const char* KetAtomId = "atomId";
        inline constexpr const char* KetAttachmentGroupId = "attachmentGroupId";

        // Attachment groups, declared by the molecule node that owns their atoms.
        inline constexpr const char* KetAttachmentGroups = "attachmentGroups";
        inline constexpr const char* KetAttachmentGroupIdField = "id";
        inline constexpr const char* KetAttachmentGroupAtoms = "atoms";

        // Prefix of a molecule node reference, as in "mol0".
        inline constexpr const char* KetMoleculePrefix = "mol";
    }
}

#endif
