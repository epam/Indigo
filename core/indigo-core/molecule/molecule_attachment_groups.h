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

#include <list>
#include <unordered_set>
#include <vector>

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/ptr_reusable_pool.h"
#include "base_cpp/reusable.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

// The attachment-group model behind haptic bonds: feature #3233, model #3837.
// Markush bonds (#3731) reuse the group with the other attachment mode.

namespace indigo
{
    // The `ATTACH=` keyword of a CTfile V3000 bond record: All reaches every atom
    // of the group at once (haptic), Any reaches one unspecified atom of it
    // (Markush). Any is recorded on load but not acted upon.
    enum class AttachmentMode
    {
        All,
        Any
    };

    // A set of atoms acting as one collective endpoint of a bond.
    // Not a graph vertex: never appears in vertices()/edges().
    // No stored position: derive it from the member atoms.
    class DLLEXPORT AttachmentGroup : public Reusable
    {
    public:
        // A bond from this group to an atom outside it. Owned by the group, so it
        // cannot outlive the endpoint it starts from.
        struct Bond
        {
            int atom;  // partner atom, index in the owning molecule
            int order; // bond order as carried by the file formats
        };

        AttachmentGroup();
        ~AttachmentGroup() override;

        AttachmentGroup(const AttachmentGroup&) = delete;
        AttachmentGroup& operator=(const AttachmentGroup&) = delete;

        // Reusable: restore the default-constructed state, keeping the buffers.
        void reuse() override;

        // Initializing form used by the pool's add(mode).
        void reuse(AttachmentMode mode);

        AttachmentMode mode() const
        {
            return _mode;
        }
        void setMode(AttachmentMode mode)
        {
            _mode = mode;
        }

        // Member atoms, in insertion order and without duplicates.
        const std::vector<int>& atoms() const
        {
            return _atoms;
        }
        bool hasAtom(int atom) const;
        void addAtom(int atom);
        void setAtoms(const std::vector<int>& atoms);

        const std::vector<Bond>& bonds() const
        {
            return _bonds;
        }
        void addBond(int atom, int order);

        // Renumbers member atoms and bond partners through `atom_mapping`, where
        // -1 means the atom is gone; bonds to a gone partner are dropped.
        // Returns false if a MEMBER atom is gone — the caller must then drop the
        // whole group, never a truncated one.
        bool remapAtoms(const Array<int>& atom_mapping);

        // Aggregate charge and radical of the pi system: the position inside the
        // system is not significant (BIOVIA Chemical Representation).
        int charge() const
        {
            return _charge;
        }
        void setCharge(int charge)
        {
            _charge = charge;
        }
        int radical() const
        {
            return _radical;
        }
        void setRadical(int radical)
        {
            _radical = radical;
        }

    private:
        void _reset(); // the one place the fields are listed; not virtual — called from the constructor

        std::vector<int> _atoms;
        std::vector<Bond> _bonds;
        AttachmentMode _mode;
        int _charge;
        int _radical;
    };

    // The attachment groups of one molecule, plus the marks telling which ordinary
    // bonds are haptic.
    //
    // Every removal path of BaseMolecule must reach this class: see
    // onAtomsRemoved() and onBondRemoved().
    class DLLEXPORT MoleculeAttachmentGroups
    {
    public:
        DECL_ERROR;

        MoleculeAttachmentGroups();
        ~MoleculeAttachmentGroups();

        MoleculeAttachmentGroups(const MoleculeAttachmentGroups&) = delete;
        MoleculeAttachmentGroups& operator=(const MoleculeAttachmentGroups&) = delete;

        int addGroup(AttachmentMode mode = AttachmentMode::All);

        AttachmentGroup& group(int idx);
        const AttachmentGroup& group(int idx) const;

        bool hasGroup(int idx) const;
        int groupCount() const;

        // Removes the group and, with it, its bonds. Indices of the other groups
        // are unaffected — callers hold group indices across such calls.
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

        void setBondHaptic(int bond_idx, bool haptic = true);
        bool isBondHaptic(int bond_idx) const;
        int hapticBondCount() const;

        // Must be called for every bond the molecule removes: the graph recycles
        // freed bond indices, so a mark left behind is inherited by the next bond
        // to take the index.
        void onBondRemoved(int bond_idx);

        void clear();
        bool isEmpty() const;

        // Copies the groups and marks of `other` that survive a submolecule
        // mapping (-1 means "dropped"), under the same all-or-nothing rule as
        // remapAtoms(). `group_mapping` receives, per source group index, the
        // index it got here or -1: the haptic bonds of the same merge address
        // their groups by index and have no other way to follow them.
        void mergeWithSubmolecule(const MoleculeAttachmentGroups& other, const Array<int>& atom_mapping, const Array<int>& bond_mapping,
                                  Array<int>& group_mapping);

        // Atom sets that must be treated as connected even though no edge joins
        // them: each group contributes its members plus the partners of its bonds.
        // Feeds Graph::countComponents(external_neighbors).
        void collectConnectivitySets(std::list<std::unordered_set<int>>& neighbors) const;

    private:
        void _checkGroup(int idx) const;

        PtrReusablePool<AttachmentGroup> _groups;
        std::unordered_set<int> _haptic_bonds;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
