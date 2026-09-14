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

#ifndef __molecule_attachment_groups__
#define __molecule_attachment_groups__

#include <vector>

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/ptr_reusable_pool.h"
#include "base_cpp/reusable.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

// The attachment groups behind haptic bonds: feature #3233, model #3837.
// The bonds themselves live in MoleculeHapticBonds, one level up.

namespace indigo
{
    // A set of atoms acting as one collective endpoint of a haptic bond.
    // Not a graph vertex: never appears in vertices()/edges().
    // No stored position: derive it from the member atoms.
    class DLLEXPORT AttachmentGroup : public Reusable
    {
    public:
        AttachmentGroup();
        ~AttachmentGroup() override;

        AttachmentGroup(const AttachmentGroup&) = delete;
        AttachmentGroup& operator=(const AttachmentGroup&) = delete;

        // Reusable: restore the default-constructed state, keeping the buffers.
        void reuse() override;

        // Member atoms, in insertion order and without duplicates.
        const std::vector<int>& atoms() const
        {
            return _atoms;
        }
        bool hasAtom(int atom) const;
        void addAtom(int atom);
        void setAtoms(const std::vector<int>& atoms);

        // Renumbers member atoms through `atom_mapping`, where -1 means the atom
        // is gone. Returns false if any member is gone — the caller must then drop
        // the whole group, never a truncated one.
        bool remapAtoms(const Array<int>& atom_mapping);

        // The atom a file uses to draw the group with: the star atom of a V3000
        // ENDPTS record. -1 when the source named none, as KET does. Not a member
        // of the group and not part of it chemically — a group whose anchor is
        // gone stays a group, and a saver that needs one puts a fresh star at the
        // centre of the members.
        int anchorAtom() const
        {
            return _anchor_atom;
        }
        void setAnchorAtom(int atom)
        {
            _anchor_atom = atom;
        }

        // Renumbers the anchor through `atom_mapping`; the anchor simply goes away
        // when its atom does, which is why this is not part of remapAtoms() and
        // never makes the caller drop the group.
        void remapAnchorAtom(const Array<int>& atom_mapping);

    private:
        void _reset(); // the one place the fields are listed; not virtual — called from the constructor

        std::vector<int> _atoms;
        int _anchor_atom;
    };

    // The attachment groups of one molecule.
    // Knows nothing of the haptic bonds that address its groups by index, so every
    // operation that drops or renumbers a group reports that through an out-param
    // and leaves propagating it to BaseMolecule.
    class DLLEXPORT MoleculeAttachmentGroups
    {
    public:
        DECL_ERROR;

        MoleculeAttachmentGroups();
        ~MoleculeAttachmentGroups();

        MoleculeAttachmentGroups(const MoleculeAttachmentGroups&) = delete;
        MoleculeAttachmentGroups& operator=(const MoleculeAttachmentGroups&) = delete;

        int addGroup();

        AttachmentGroup& group(int idx);
        const AttachmentGroup& group(int idx) const;

        bool hasGroup(int idx) const;
        int groupCount() const;

    private:
        friend class BaseMolecule;
        // Indices of the other groups are unaffected — callers hold group indices
        // across such calls.
        void removeGroup(int idx);

    public:
        // Takes the mapping BaseMolecule::removeAtoms builds (-1 = removed). A group
        // that loses a member is dropped whole, never truncated; `removed_groups`
        // receives the indices of those dropped.
        void onAtomsRemoved(const Array<int>& atom_mapping, Array<int>& removed_groups);

        // Index walk over the live groups, mirroring the pool: end() is one
        // past the last slot, not the number of groups.
        int begin() const;
        int next(int idx) const;
        int end() const;

        void clear();
        bool isEmpty() const;

        // Copies the groups of `other` that survive `atom_mapping` (-1 = dropped),
        // under the same all-or-nothing rule as remapAtoms(). `group_mapping`
        // receives, per source group index, the index it got here or -1.
        void mergeWithSubmolecule(const MoleculeAttachmentGroups& other, const Array<int>& atom_mapping, Array<int>& group_mapping);

    private:
        void _checkGroup(int idx) const;

        PtrReusablePool<AttachmentGroup> _groups;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
