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

#ifndef __molecule_electrons_localizer__
#define __molecule_electrons_localizer__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/obj.h"
#include "base_cpp/tlscont.h"
#include "graph/graph_constrained_bmatching_finder.h"

namespace indigo
{

    class BaseMolecule;
    class Molecule;
    class Graph;

    // This class localizes specified electrons on molecule skeleton.
    // Localization parameters are: number of double bonds and
    // number of primary and secondary lonepairs.
    // The more secondary lonepairs are specified, the bigger charge
    // is assigned to the atom. To minimize the number of atoms with
    // charges user should find valid configuration with minimum
    // number of secondary lonepairs.
    //
    // During localization only atom labels, radicals, and implicit
    // hydrogens are taken into account.
    //
    // The algorithm works only with p-orbitals. Currently, atoms
    // with valences more than 4 can't be handled.
    //
    // * Implementation details *
    // For skeleton specified algorithm construct extended skeleton by
    // adding and connecting lonepairs node to each atom. Every valid electron
    // configuration is mapped to the b-matching in the extended skeleton.
    // Atom capacity in b-matching is divided into two groups: primary and secondary.
    // If the atom is saturated with primary capacity and have zero value with secondary
    // capapacity, then zero charge is assigned to the atom. If atom is saturated with primary
    // and secondary capacities, the full octet is assigned to the atom.
    class MoleculeElectronsLocalizer
    {
    public:
        MoleculeElectronsLocalizer(Molecule& skeleton);

        void setParameters(int double_bonds, int primary_lonepairs, int secondary_lonepairs);

        // Perform localization and return true if localization is possible
        // If 'only_check_possibility' is true that possibility of localization
        // with specified double bonds and lonepairs is checked without dividing
        // the lonepairs into primary and secondary groups.
        bool localize(bool only_check_possibility = false);

        // Copy localized bonds and charges infomation to the specifed molecule.
        // 'dest' molecule must have the same structure as 'skeleton' molecule.
        void copyBondsAndCharges(Molecule& dest, const Array<int>& mapping) const;

        // Methods for adding constraints for atom charges and bonds.
        // For better performance fix bonds first.
        bool fixAtomCharge(int atom, int charge);
        bool fixAtomConnectivity(int atom, int connectivity);
        bool fixBond(int bond, int type);
        bool fixBondSingleDouble(int bond);
        void unfixAll();

        void unfixAtom(int atom);
        void unfixBond(int bond);

        int getZeroChargeLonepairs() const
        {
            return _zc_lonepairs;
        }
        int getLocalizationChargesCount() const;
        bool isAllAtomsHaveOctet() const;

        DECL_ERROR;

    private:
        void _construct();
        void _constructBMatchingFinder();
        // Calculate max_connectivity, zc_connectivity and zc_lonepairs
        void _setupAtomProperties();

        void _setupBMatchingNodes();
        void _setupBMatchingEdges();

        void _setupBMatchingNode(int atom);
        void _setupBMatchingNodeAtom(int atom);
        void _setupBMatchingNodeOrbital(int atom);

        bool _fixAtomConnectivityAndLonepairs(int atom, int added_connectivity, int lonepairs);
        void _unfixAtomConnectivityAndLonepairs(int atom);

        void _fixAtomSaturated(int atom);
        void _unfixAtomSaturated(int atom);

        // Split connectivity (lonepairs) into primary and secondary
        void _splitConnectivity(int atom, int conn, int* prim, int* sec) const;
        void _splitLonepairs(int atom, int lonepairs, int* prim, int* sec) const;

        // Calculate connectivity and number of lonepairs
        // for specifed number of electrons
        bool _calcConnectivityAndLoneparis(int atom, int charge, int* conn, int* lp);

        // Check whether atom can atom be not saturated
        bool _canAtomBeUnsaturated(int atom);

        // Find localization with all atom localized validly
        bool _findValidSolution(int cardinality);

        // Check is electrons localization for atom is valid
        enum
        {
            OK = 0,
            LONEPAIRS,
            CONNECTIVITY
        };
        int _isLocalizationValid(int atom) const;

        bool _branchOnLonepairs(int cardinality, int invalid_atom);
        bool _branchOnConnectivity(int cardinality, int invalid_atom);

        // Set constraint set parameters for atoms and lonepairs
        bool _setConstraintSetForAtoms();
        bool _setConstraintSetForLonepairs(bool only_check_possibility);

        void _updateAtomBondFixed(int atom, int bond_type, bool fixed);
        bool _checkAtomBondFixed(int atom, int bond_type);

        int _getAtomCharge(int atom) const;
        void _getAtomConnAndLonepairs(int atom, int& added_conn, int& lonepairs) const;

        Obj<GraphConstrainedBMatchingFinder> _finder;
        Molecule& _skeleton;

        // Localization parameters
        int _double_bonds, _primary_lonepairs, _secondary_lonepairs;
        // Nodes constraint sets for constrained b-matching finder
        enum
        {
            _PRIMARY_ATOMS_SET,
            _SECONDARY_ATOMS_SET,
            _SUM_ATOMS_SET,
            _CONSTRAINED_ATOMS_SET,
            _PRIMARY_LONEPAIRS_SET,
            _SECONDARY_LONEPAIRS_SET,
            _CONSTRAINED_LONEPAIRS_SET,
            _SUM_LONEPAIRS_SET,
            _SET_MAX
        };

        struct _AtomInfo
        {
            int atom_node, orbitals_node;
            int orbitals_edge;

            // Atom have fixed number of lonepairs and connectivity
            bool atom_fixed;
            bool atom_connectivity_fixed;
            // Atoms in 6-th and 7-th groups must be saturated in any localization.
            // This state is true if atom must have full octet in any localization.
            bool atom_saturated;

            // Current maximum additional connectivity (atom capacity)
            int max_add_connectivity;
            // Maximum additional connectivity without constrained bonds
            int max_add_connectivity0;
            // Connectivity and lonepairs for atom with zero charge (zc_)
            int zc_connectivity, zc_lonepairs;

            // Information for fixed atoms
            int fixed_connectivity, fixed_lonepairs;

            // Connectivity with skeleton + number of bonds fixed
            int skeleton_connectivity;
        };

        // Summary double bonds and lonepairs for zero charge
        int _zc_atoms_connectivity, _zc_lonepairs;

        // Constraints parameters
        int _constrained_primary_double_bonds_conn, _constrained_secondary_double_bonds_conn, _constrained_primary_lonepairs, _constrained_secondary_lonepairs,
            _constrained_primary_atoms, _constrained_secondary_atoms, _constrained_saturated_atoms;

        CP_DECL;
        // Molecule skeleton with orbitals nodes attached
        TL_CP_DECL(Graph, _extended_skeleton);
        // Edge mapping between skeleton and extended skeleton
        TL_CP_DECL(Array<int>, _edge_mapping);
        // Additional information per atom for extracting data from
        // matching in extended skeleton graph
        TL_CP_DECL(Array<_AtomInfo>, _atom_info);
        // Array with fixed edges
        TL_CP_DECL(Array<int>, _edges_fixed_type);
    };

} // namespace indigo

#endif // __molecule_electrons_localizer__
