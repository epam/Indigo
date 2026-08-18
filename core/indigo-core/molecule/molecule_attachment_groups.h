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
        // Attributes of the anchor atom that the file formats put the charge and
        // the radical of the pi system on, and which is absorbed on loading.
        // Carried through unchanged: never recomputed and never spread over the
        // member atoms, so a structure charged either way comes back as it came in.
        struct Anchor
        {
            int charge = 0;
            int radical = 0;
        };

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

        const Anchor& anchor() const
        {
            return _anchor;
        }
        void setAnchor(const Anchor& anchor)
        {
            _anchor = anchor;
        }

    private:
        void _reset(); // the one place the fields are listed; not virtual — called from the constructor

        std::vector<int> _atoms;
        Anchor _anchor;
    };

    // The attachment groups of one molecule.
    //
    // Every removal path of BaseMolecule must reach this class: see
    // onAtomsRemoved(). A single group is removed through
    // BaseMolecule::removeAttachmentGroup(), which also drops the haptic bonds
    // that address it.
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

        // Removes the group. Indices of the other groups are unaffected — callers
        // hold group indices across such calls.
        void removeGroup(int idx);

        // Must be called when the molecule removes atoms, with the same mapping
        // BaseMolecule::removeAtoms builds (-1 = removed). Groups that lose a
        // member are dropped whole; their indices are reported in `removed_groups`
        // because the haptic bonds that reference them must go too, and this
        // container does not know about them.
        void onAtomsRemoved(const Array<int>& atom_mapping, Array<int>& removed_groups);

        // Index walk over the live groups, mirroring the pool: end() is one
        // past the last slot, not the number of groups.
        int begin() const;
        int next(int idx) const;
        int end() const;

        void clear();
        bool isEmpty() const;

        // Copies the groups of `other` that survive a submolecule mapping (-1
        // means "dropped"), under the same all-or-nothing rule as remapAtoms().
        // `group_mapping` receives, per source group index, the index it got here
        // or -1: the haptic bonds of the same merge address their groups by index
        // and have no other way to follow them.
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
