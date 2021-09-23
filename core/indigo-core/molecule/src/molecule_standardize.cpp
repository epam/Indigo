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

#include "molecule/molecule_standardize.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_standardize_options.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeStandardizer, "Molecule Standardizer");

CP_DEF(MoleculeStandardizer);
MoleculeStandardizer::MoleculeStandardizer() : CP_INIT
{
}

bool MoleculeStandardizer::standardize(Molecule& mol, const StandardizeOptions& options)
{
    if (options.standardize_stereo)
    {
        _standardizeStereo(mol);
    }

    if (options.standardize_charges)
    {
        _standardizeCharges(mol);
    }

    if (options.center_molecule)
    {
        _centerMolecule(mol);
    }

    if (options.remove_single_atom_fragments)
    {
        _removeSingleAtomFragments(mol);
    }

    if (options.keep_smallest_fragment)
    {
        _keepSmallestFragment(mol);
    }

    if (options.keep_largest_fragment)
    {
        _keepLargestFragment(mol);
    }

    if (options.remove_largest_fragment)
    {
        _removeLargestFragment(mol);
    }

    if (options.make_non_h_atoms_c_atoms)
    {
        _makeNonHAtomsCAtoms(mol);
    }

    if (options.make_non_h_atoms_a_atoms)
    {
        _makeNonHAtomsAAtoms(mol);
    }

    if (options.make_non_c_h_atoms_q_atoms)
    {
        _makeNonCHAtomsQAtoms(mol);
    }

    if (options.make_all_bonds_single)
    {
        _makeAllBondsSingle(mol);
    }

    if (options.clear_coordinates)
    {
        _clearCoordinates(mol);
    }

    if (options.fix_coordinate_dimension)
    {
        _fixCoordinateDimension(mol);
    }

    if (options.straighten_triple_bonds)
    {
        _straightenTripleBonds(mol);
    }

    if (options.straighten_allenes)
    {
        _straightenAllenes(mol);
    }

    if (options.clear_molecule)
    {
        _clearMolecule(mol);
    }

    if (options.remove_molecule)
    {
        _removeMolecule(mol);
    }

    if (options.clear_stereo)
    {
        _clearStereo(mol);
    }

    if (options.clear_enhanced_stereo)
    {
        _clearEnhancedStereo(mol);
    }

    if (options.clear_unknown_stereo)
    {
        _clearUnknownStereo(mol);
    }

    if (options.clear_unknown_atom_stereo)
    {
        _clearUnknownAtomStereo(mol);
    }

    if (options.clear_unknown_cis_trans_bond_stereo)
    {
        _clearUnknownCisTransBondStereo(mol);
    }

    if (options.clear_cis_trans_bond_stereo)
    {
        _clearCisTransBondStereo(mol);
    }

    if (options.set_stereo_from_coordinates)
    {
        _setStereoFromCoordinates(mol);
    }

    if (options.reposition_stereo_bonds)
    {
        _repositionStereoBonds(mol);
    }

    if (options.reposition_axial_stereo_bonds)
    {
        _repositionAxialStereoBonds(mol);
    }

    if (options.fix_direction_of_wedge_bonds)
    {
        _fixDirectionOfWedgeBonds(mol);
    }

    if (options.clear_charges)
    {
        _clearCharges(mol);
    }

    if (options.clear_pi_bonds)
    {
        _clearPiBonds(mol);
    }

    if (options.clear_highlight_colors)
    {
        _clearHighlightColors(mol);
    }

    if (options.clear_query_info)
    {
        _clearQueryInfo(mol);
    }

    if (options.clear_atom_labels)
    {
        _clearAtomLabels(mol);
    }

    if (options.clear_bond_labels)
    {
        _clearBondLabels(mol);
    }

    if (options.neutralize_bonded_zwitterions)
    {
        _neutralizeBondedZwitterions(mol);
    }

    if (options.clear_unusual_valence)
    {
        _clearUnusualValence(mol);
    }

    if (options.clear_isotopes)
    {
        _clearIsotopes(mol);
    }

    if (options.clear_dative_bonds)
    {
        _clearDativeBonds(mol);
    }

    if (options.clear_hydrogen_bonds)
    {
        _clearHydrogenBonds(mol);
    }

    if (options.localize_markush_r_atoms_on_rings)
    {
        _localizeMarkushRAtomsOnRings(mol);
    }

    if (options.create_coordination_bonds)
    {
        _createCoordinationBonds(mol);
    }

    if (options.create_hydrogen_bonds)
    {
        _createHydrogenBonds(mol);
    }

    if (options.remove_extra_stereo_bonds)
    {
        _removeExtraStereoBonds(mol);
    }

    return true;
}

bool MoleculeStandardizer::standardize(QueryMolecule& query, const StandardizeOptions& options)
{
    if (options.standardize_stereo)
    {
        _standardizeStereo(query);
    }

    if (options.standardize_charges)
    {
        _standardizeCharges(query);
    }

    if (options.center_molecule)
    {
        _centerMolecule(query);
    }

    if (options.remove_single_atom_fragments)
    {
        _removeSingleAtomFragments(query);
    }

    if (options.keep_smallest_fragment)
    {
        _keepSmallestFragment(query);
    }

    if (options.keep_largest_fragment)
    {
        _keepLargestFragment(query);
    }

    if (options.remove_largest_fragment)
    {
        _removeLargestFragment(query);
    }

    if (options.make_non_h_atoms_c_atoms)
    {
        _makeNonHAtomsCAtoms(query);
    }

    if (options.make_non_h_atoms_a_atoms)
    {
        _makeNonHAtomsAAtoms(query);
    }

    if (options.make_non_c_h_atoms_q_atoms)
    {
        _makeNonCHAtomsQAtoms(query);
    }

    if (options.make_all_bonds_single)
    {
        _makeAllBondsSingle(query);
    }

    if (options.clear_coordinates)
    {
        _clearCoordinates(query);
    }

    if (options.fix_coordinate_dimension)
    {
        _fixCoordinateDimension(query);
    }

    if (options.straighten_triple_bonds)
    {
        _straightenTripleBonds(query);
    }

    if (options.straighten_allenes)
    {
        _straightenAllenes(query);
    }

    if (options.clear_molecule)
    {
        _clearMolecule(query);
    }

    if (options.remove_molecule)
    {
        _removeMolecule(query);
    }

    if (options.clear_stereo)
    {
        _clearStereo(query);
    }

    if (options.clear_enhanced_stereo)
    {
        _clearEnhancedStereo(query);
    }

    if (options.clear_unknown_stereo)
    {
        _clearUnknownStereo(query);
    }

    if (options.clear_unknown_atom_stereo)
    {
        _clearUnknownAtomStereo(query);
    }

    if (options.clear_unknown_cis_trans_bond_stereo)
    {
        _clearUnknownCisTransBondStereo(query);
    }

    if (options.clear_cis_trans_bond_stereo)
    {
        _clearCisTransBondStereo(query);
    }

    if (options.set_stereo_from_coordinates)
    {
        _setStereoFromCoordinates(query);
    }

    if (options.reposition_stereo_bonds)
    {
        _repositionStereoBonds(query);
    }

    if (options.reposition_axial_stereo_bonds)
    {
        _repositionAxialStereoBonds(query);
    }

    if (options.fix_direction_of_wedge_bonds)
    {
        _fixDirectionOfWedgeBonds(query);
    }

    if (options.clear_charges)
    {
        _clearCharges(query);
    }

    if (options.clear_pi_bonds)
    {
        _clearPiBonds(query);
    }

    if (options.clear_highlight_colors)
    {
        _clearHighlightColors(query);
    }

    if (options.clear_query_info)
    {
        _clearQueryInfo(query);
    }

    if (options.clear_atom_labels)
    {
        _clearAtomLabels(query);
    }

    if (options.clear_bond_labels)
    {
        _clearBondLabels(query);
    }

    if (options.neutralize_bonded_zwitterions)
    {
        _neutralizeBondedZwitterions(query);
    }

    if (options.clear_unusual_valence)
    {
        _clearUnusualValence(query);
    }

    if (options.clear_isotopes)
    {
        _clearIsotopes(query);
    }

    if (options.clear_dative_bonds)
    {
        _clearDativeBonds(query);
    }

    if (options.clear_hydrogen_bonds)
    {
        _clearHydrogenBonds(query);
    }

    if (options.localize_markush_r_atoms_on_rings)
    {
        _localizeMarkushRAtomsOnRings(query);
    }

    if (options.create_coordination_bonds)
    {
        _createCoordinationBonds(query);
    }

    if (options.create_hydrogen_bonds)
    {
        _createHydrogenBonds(query);
    }

    return true;
}

void MoleculeStandardizer::_standardizeStereo(Molecule& mol)
{
    QS_DEF(Array<int>, ignored);

    ignored.clear_resize(mol.vertexEnd());
    ignored.zerofill();

    for (auto i : mol.vertices())
        if (mol.convertableToImplicitHydrogen(i))
            ignored[i] = 1;

    MoleculeAutomorphismSearch as;

    as.detect_invalid_cistrans_bonds = true;
    as.detect_invalid_stereocenters = true;
    as.find_canonical_ordering = false;
    as.ignored_vertices = ignored.ptr();
    as.process(mol);

    for (auto i : mol.vertices())
    {
        if ((mol.isPossibleStereocenter(i)) && !as.invalidStereocenter(i))
        {
            if (mol.stereocenters.exists(i))
                continue;
            else
                mol.addStereocenters(i, MoleculeStereocenters::ATOM_ANY, 0, false);
        }
        else
        {
            if (mol.stereocenters.exists(i))
                mol.stereocenters.setType(i, 0, 0);
            else
                continue;
        }
    }

    for (auto i : mol.edges())
    {
        if (!as.invalidCisTransBond(i))
        {
            if (mol.cis_trans.getParity(i) > 0)
                continue;
            else if (mol.registerBondAndSubstituentsCisTrans(i))
            {
                mol.setBondDirection(i, BOND_EITHER);
                mol.cis_trans.setParity(i, 0);
            }
        }
        else
        {
            if (mol.cis_trans.getParity(i) > 0)
                mol.cis_trans.setParity(i, 0);
            else
                continue;
        }
    }
}

void MoleculeStandardizer::_standardizeStereo(QueryMolecule& mol)
{
    throw Error("This option is not available for QueryMolecule object");
}

void MoleculeStandardizer::_standardizeCharges(Molecule& mol)
{
    for (auto i : mol.vertices())
    {
        switch (mol.getAtomNumber(i))
        {
        case ELEM_N:
            if (_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 4)
            {
                mol.setAtomCharge(i, +1);
                if (_getNumberOfBonds(mol, i, BOND_SINGLE, true, ELEM_O) == 1)
                {
                    const Vertex& v = mol.getVertex(i);
                    for (int j : v.neighbors())
                    {
                        if ((mol.getAtomNumber(v.neiVertex(j)) == ELEM_O) && (mol.getVertex(v.neiVertex(j)).degree() == 1))
                            mol.setAtomCharge(v.neiVertex(j), -1);
                    }
                }
            }
            else if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 1) && (_getNumberOfBonds(mol, i, BOND_AROMATIC, false, 0) == 2))
            {
                mol.setAtomCharge(i, +1);
                if (_getNumberOfBonds(mol, i, BOND_SINGLE, true, ELEM_O) == 1)
                {
                    const Vertex& v = mol.getVertex(i);
                    for (int j : v.neighbors())
                    {
                        if ((mol.getAtomNumber(v.neiVertex(j)) == ELEM_O) && (mol.getVertex(v.neiVertex(j)).degree() == 1))
                            mol.setAtomCharge(v.neiVertex(j), -1);
                    }
                }
            }
            else if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 1) && (_getNumberOfBonds(mol, i, BOND_DOUBLE, false, 0) == 2) &&
                     (_getNumberOfBonds(mol, i, BOND_DOUBLE, true, ELEM_O) == 2))
            {
                mol.setAtomCharge(i, +1);
                const Vertex& v = mol.getVertex(i);
                for (int j : v.neighbors())
                {
                    if ((mol.getAtomNumber(v.neiVertex(j)) == ELEM_O) && (mol.getVertex(v.neiVertex(j)).degree() == 1))
                    {
                        if (mol.getBondOrder(v.neiEdge(j) == BOND_DOUBLE))
                        {
                            mol.setAtomCharge(v.neiVertex(j), -1);
                            mol.setBondOrder(v.neiEdge(j), BOND_SINGLE);
                            break;
                        }
                    }
                }
            }
            else if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 2) && (_getNumberOfBonds(mol, i, BOND_DOUBLE, false, 0) == 1))
            {
                mol.setAtomCharge(i, +1);
            }
            break;
        case ELEM_O:
            if (mol.getVertex(i).degree() == 3)
            {
                mol.setAtomCharge(i, +1);
            }
            else if (mol.getVertex(i).degree() == 2)
            {
                if (_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 2)
                    break;
                if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, ELEM_C) == 1) && (_getNumberOfBonds(mol, i, BOND_DOUBLE, false, 0) == 1))
                {
                    mol.setAtomCharge(i, +1);
                }
            }
            break;
        case ELEM_S:
            if (mol.getVertex(i).degree() == 3)
            {
                mol.setAtomCharge(i, +1);
            }
            else if (mol.getVertex(i).degree() == 2)
            {
                if (_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 2)
                    break;
                if ((_getNumberOfBonds(mol, i, BOND_SINGLE, false, ELEM_C) == 1) && (_getNumberOfBonds(mol, i, BOND_DOUBLE, false, 0) == 1))
                {
                    mol.setAtomCharge(i, +1);
                }
            }
            break;
        case ELEM_F:
            if (mol.getVertex(i).degree() == 0)
            {
                mol.setAtomCharge(i, -1);
            }
            break;
        case ELEM_Cl:
            if (mol.getVertex(i).degree() == 0)
            {
                mol.setAtomCharge(i, -1);
            }
            break;
        case ELEM_Br:
            if (mol.getVertex(i).degree() == 0)
            {
                mol.setAtomCharge(i, -1);
            }
            break;
        case ELEM_I:
            if (mol.getVertex(i).degree() == 0)
            {
                mol.setAtomCharge(i, -1);
            }
            break;

        default:
            break;
        }
    }
}

void MoleculeStandardizer::_standardizeCharges(QueryMolecule& mol)
{
    throw Error("This option is not available for QueryMolecule object");
}

void MoleculeStandardizer::_centerMolecule(BaseMolecule& mol)
{
    if (!Molecule::hasCoord(mol))
        throw Error("Atoms coordinates are not defined");

    Vec3f mol_min = Vec3f(INFINITY, INFINITY, INFINITY);
    Vec3f mol_max = Vec3f(-INFINITY, -INFINITY, -INFINITY);
    Vec3f mol_center = Vec3f(0, 0, 0);

    for (auto i : mol.vertices())
    {
        Vec3f& xyz = mol.getAtomXyz(i);
        if (xyz.x < mol_min.x)
            mol_min.x = xyz.x;
        if (xyz.y < mol_min.y)
            mol_min.y = xyz.y;
        if (xyz.z < mol_min.z)
            mol_min.z = xyz.z;

        if (xyz.x > mol_max.x)
            mol_max.x = xyz.x;
        if (xyz.y > mol_max.y)
            mol_max.y = xyz.y;
        if (xyz.z > mol_max.z)
            mol_max.z = xyz.z;
    }

    mol_center.x = (mol_min.x + mol_max.x) / 2;
    mol_center.y = (mol_min.y + mol_max.y) / 2;
    mol_center.z = (mol_min.z + mol_max.z) / 2;

    for (auto i : mol.vertices())
    {
        Vec3f& xyz = mol.getAtomXyz(i);
        xyz.x -= mol_center.x;
        xyz.y -= mol_center.y;
        xyz.z -= mol_center.z;
        mol.setAtomXyz(i, xyz);
    }
}

void MoleculeStandardizer::_removeSingleAtomFragments(BaseMolecule& mol)
{
    QS_DEF(Array<int>, single_atoms);
    single_atoms.clear();

    for (auto i : mol.vertices())
    {
        auto atom_number = mol.getAtomNumber(i);
        if (atom_number != ELEM_H)
        {
            if (mol.getVertex(i).degree() == 0)
                single_atoms.push(i);
        }
    }

    if (single_atoms.size() > 0)
        mol.removeAtoms(single_atoms);
}

void MoleculeStandardizer::_keepSmallestFragment(BaseMolecule& mol)
{
    if (mol.vertexCount() <= 1)
        return;

    auto ncomp = mol.countComponents();
    if (ncomp == 1)
        return;

    auto min_size = mol.vertexCount();
    auto min_comp = 0;
    for (auto i = 0; i < ncomp; i++)
    {
        if (mol.countComponentVertices(i) < min_size)
        {
            min_size = mol.countComponentVertices(i);
            min_comp = i;
        }
    }

    QS_DEF(Array<int>, remove_atoms);
    remove_atoms.clear();

    for (auto i : mol.vertices())
    {
        if (mol.vertexComponent(i) != min_comp)
            remove_atoms.push(i);
    }

    if (remove_atoms.size() > 0)
        mol.removeAtoms(remove_atoms);
}

void MoleculeStandardizer::_keepLargestFragment(BaseMolecule& mol)
{

    if (mol.vertexCount() <= 1)
        return;

    auto ncomp = mol.countComponents();
    if (ncomp == 1)
        return;

    auto max_size = 0;
    auto max_comp = 0;
    for (auto i = 0; i < ncomp; i++)
    {
        if (mol.countComponentVertices(i) > max_size)
        {
            max_size = mol.countComponentVertices(i);
            max_comp = i;
        }
    }

    QS_DEF(Array<int>, remove_atoms);
    remove_atoms.clear();

    for (auto i : mol.vertices())
    {
        if (mol.vertexComponent(i) != max_comp)
            remove_atoms.push(i);
    }

    if (remove_atoms.size() > 0)
        mol.removeAtoms(remove_atoms);
}

void MoleculeStandardizer::_removeLargestFragment(BaseMolecule& mol)
{
    if (mol.vertexCount() <= 1)
        return;

    auto ncomp = mol.countComponents();
    if (ncomp == 1)
        return;

    auto max_size = 0;
    auto max_comp = 0;
    for (auto i = 0; i < ncomp; i++)
    {
        if (mol.countComponentVertices(i) > max_size)
        {
            max_size = mol.countComponentVertices(i);
            max_comp = i;
        }
    }

    QS_DEF(Array<int>, remove_atoms);
    remove_atoms.clear();

    for (auto i : mol.vertices())
    {
        if (mol.vertexComponent(i) == max_comp)
            remove_atoms.push(i);
    }

    if (remove_atoms.size() > 0)
        mol.removeAtoms(remove_atoms);
}

void MoleculeStandardizer::_makeNonHAtomsCAtoms(Molecule& mol)
{
    for (auto i : mol.vertices())
    {
        auto atom_number = mol.getAtomNumber(i);
        if ((atom_number > ELEM_H) && (atom_number < ELEM_MAX) && (atom_number != ELEM_C))
            mol.resetAtom(i, ELEM_C);
    }
}

void MoleculeStandardizer::_makeNonHAtomsCAtoms(QueryMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        auto atom_number = mol.getAtomNumber(i);
        if ((atom_number != ELEM_H) && (atom_number != ELEM_C))
            mol.resetAtom(i, new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C));
    }
}

void MoleculeStandardizer::_makeNonHAtomsAAtoms(Molecule& mol)
{
    throw Error("This option is available only for QueryMolecule object");
}

void MoleculeStandardizer::_makeNonHAtomsAAtoms(QueryMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        auto atom_number = mol.getAtomNumber(i);
        if (atom_number != ELEM_H)
            mol.resetAtom(i, QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)));
    }
}

void MoleculeStandardizer::_makeNonCHAtomsQAtoms(Molecule& mol)
{
    throw Error("This option is available only for QueryMolecule object");
}

void MoleculeStandardizer::_makeNonCHAtomsQAtoms(QueryMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        auto atom_number = mol.getAtomNumber(i);
        if ((atom_number != ELEM_H) && (atom_number != ELEM_C))
            mol.resetAtom(i, QueryMolecule::Atom::und(QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                                                      QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C))));
    }
}

void MoleculeStandardizer::_makeAllBondsSingle(Molecule& mol)
{
    for (auto i : mol.edges())
    {
        if (mol.getBondOrder(i) != BOND_SINGLE)
            mol.setBondOrder(i, BOND_SINGLE, false);
    }
}

void MoleculeStandardizer::_makeAllBondsSingle(QueryMolecule& mol)
{
    for (auto i : mol.edges())
    {
        if (mol.getBondOrder(i) != BOND_SINGLE)
            mol.resetBond(i, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE));
    }
}

void MoleculeStandardizer::_clearCoordinates(BaseMolecule& mol)
{
    mol.clearXyz();
}

void MoleculeStandardizer::_fixCoordinateDimension(BaseMolecule& mol)
{
    throw Error("This option is not used for Indigo");
}

void MoleculeStandardizer::_straightenTripleBonds(BaseMolecule& mol)
{
    if (!Molecule::hasCoord(mol) || (mol.vertexCount() < 2))
        throw Error("Atoms coordinates are not defined or too few atoms");

    for (auto i : mol.vertices())
    {
        if ((mol.getVertex(i).degree() == 2) && (_getNumberOfBonds(mol, i, BOND_TRIPLE, false, 0) == 1))
        {
            if (!isFragmentLinear(mol, i))
                _linearizeFragment(mol, i);
        }
    }
}

void MoleculeStandardizer::_straightenAllenes(BaseMolecule& mol)
{
    if (!Molecule::hasCoord(mol) || (mol.vertexCount() < 3))
        throw Error("Atoms coordinates are not defined or too few atoms");

    for (auto i : mol.vertices())
    {
        if ((mol.getAtomNumber(i) == ELEM_C) && (mol.getVertex(i).degree() == 2) && (_getNumberOfBonds(mol, i, BOND_DOUBLE, true, ELEM_C) == 2))
        {
            if (!isFragmentLinear(mol, i))
                _linearizeFragment(mol, i);
        }
    }
}

void MoleculeStandardizer::_clearMolecule(BaseMolecule& mol)
{
    mol.clear();
}

void MoleculeStandardizer::_removeMolecule(BaseMolecule& mol)
{
    throw Error("Not implemented yet");
}

void MoleculeStandardizer::_clearStereo(BaseMolecule& mol)
{
    mol.stereocenters.clear();

    for (auto i : mol.edges())
    {
        if (mol.getBondDirection(i) > 0)
            mol.setBondDirection(i, 0);
    }

    if (mol.cis_trans.exists())
    {
        for (auto i : mol.edges())
        {
            if (mol.cis_trans.getParity(i) > 0)
                mol.cis_trans.setParity(i, 0);
        }
    }

    mol.allene_stereo.clear();
}

void MoleculeStandardizer::_clearEnhancedStereo(BaseMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        if (mol.stereocenters.exists(i))
        {
            mol.stereocenters.setType(i, 0, 0);
        }
    }
}

void MoleculeStandardizer::_clearUnknownStereo(BaseMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        if (mol.stereocenters.exists(i) && (mol.stereocenters.getType(i) == MoleculeStereocenters::ATOM_ANY))
            mol.stereocenters.setType(i, 0, 0);
    }

    for (auto i : mol.edges())
    {
        if (mol.getBondDirection(i) == BOND_EITHER)
            mol.setBondDirection(i, 0);
    }
}

void MoleculeStandardizer::_clearUnknownAtomStereo(BaseMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        if (mol.stereocenters.exists(i) && (mol.stereocenters.getType(i) == MoleculeStereocenters::ATOM_ANY))
            mol.stereocenters.setType(i, 0, 0);
    }
}

void MoleculeStandardizer::_clearUnknownCisTransBondStereo(BaseMolecule& mol)
{
    if (mol.cis_trans.exists())
    {
        for (auto i : mol.edges())
        {
            if ((mol.cis_trans.getParity(i) == 0) && (mol.getBondDirection(i) == BOND_EITHER))
            {
                mol.setBondDirection(i, 0);
            }
        }
    }
}

void MoleculeStandardizer::_clearCisTransBondStereo(BaseMolecule& mol)
{
    if (mol.cis_trans.exists())
    {
        for (auto i : mol.edges())
        {
            if (mol.cis_trans.getParity(i) > 0)
            {
                mol.setBondDirection(i, BOND_EITHER);
                mol.cis_trans.setParity(i, 0);
            }
        }
    }
}

void MoleculeStandardizer::_setStereoFromCoordinates(BaseMolecule& mol)
{
    if (!Molecule::hasCoord(mol))
        throw Error("Atoms coordinates are not defined");

    mol.stereocenters.clear();
    mol.cis_trans.clear();
    mol.allene_stereo.clear();

    StereocentersOptions options;
    QS_DEF(Array<int>, sensible_bond_orientations);
    sensible_bond_orientations.clear_resize(mol.vertexEnd());

    mol.buildFromBondsStereocenters(options, sensible_bond_orientations.ptr());
    mol.buildFromBondsAlleneStereo(options.ignore_errors, sensible_bond_orientations.ptr());
    mol.buildCisTrans(0);

    if (mol.stereocenters.size() == 0)
        mol.buildFrom3dCoordinatesStereocenters(options);
}

void MoleculeStandardizer::_repositionStereoBonds(BaseMolecule& mol)
{
    if (!Molecule::hasCoord(mol))
        throw Error("Atoms coordinates are not defined");

    mol.markBondsStereocenters();
}

void MoleculeStandardizer::_repositionAxialStereoBonds(BaseMolecule& mol)
{
    if (!Molecule::hasCoord(mol))
        throw Error("Atoms coordinates are not defined");

    mol.markBondsAlleneStereo();
}

void MoleculeStandardizer::_fixDirectionOfWedgeBonds(BaseMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        if (!mol.stereocenters.exists(i))
        {
            const Vertex& vertex = mol.getVertex(i);

            for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                if (mol.getBondDirection2(i, vertex.neiVertex(j)) > 0)
                    mol.setBondDirection(vertex.neiEdge(j), 0);
        }
        else if (mol.vertexInRing(i))
        {
            const Vertex& vertex = mol.getVertex(i);
            int from = -1;
            int to = -1;
            int bond_dir = 0;

            for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
            {
                if ((mol.getBondDirection2(i, vertex.neiVertex(j)) > 0) && mol.vertexInRing(vertex.neiVertex(j)))
                {
                    from = j;
                    bond_dir = mol.getBondDirection2(i, vertex.neiVertex(j));
                    break;
                }
            }

            if (from == -1)
                continue;

            for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
            {
                if ((mol.getBondDirection2(i, vertex.neiVertex(j)) == 0) && !mol.vertexInRing(vertex.neiVertex(j)))
                {
                    to = j;
                    break;
                }
            }
            if ((from != -1) && (to != -1))
            {
                mol.setBondDirection(vertex.neiEdge(from), 0);
                mol.setBondDirection(vertex.neiEdge(to), bond_dir);
            }
        }
    }
}

void MoleculeStandardizer::_removeExtraStereoBonds(BaseMolecule& mol)
{
    QS_DEF(Array<int>, stereo_neibs);

    if (!Molecule::hasCoord(mol))
        throw Error("Atoms coordinates are not defined");

    // Clear wrong stereo
    for (auto i : mol.vertices())
    {
        if ((mol.stereocenters.exists(i)) && !mol.isPossibleStereocenter(i))
            mol.stereocenters.remove(i);

        if (!mol.stereocenters.exists(i))
        {
            const Vertex& vertex = mol.getVertex(i);

            for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                if (mol.getBondDirection2(i, vertex.neiVertex(j)) > 0)
                    mol.setBondDirection(vertex.neiEdge(j), 0);
        }
    }

    // Check several stereo bonds from one stereocenter
    for (auto i : mol.vertices())
    {
        int count_bonds = 0;
        stereo_neibs.clear();
        if (mol.stereocenters.exists(i))
        {
            const Vertex& vertex = mol.getVertex(i);

            for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
            {
                if (mol.getBondDirection2(i, vertex.neiVertex(j)) > 0)
                {
                    count_bonds++;
                    stereo_neibs.push(vertex.neiVertex(j));
                }
            }
        }
        // Remove unnecessary bonds
        if (count_bonds > 1)
        {
            stereo_neibs.qsort(_asc_cmp_cb, &mol);
            for (int j = 1; j < stereo_neibs.size(); j++)
            {
                int rem_edge_dir = mol.findEdgeIndex(i, stereo_neibs[j]);
                mol.setBondDirection(rem_edge_dir, 0);
            }
        }
    }
}

int MoleculeStandardizer::_asc_cmp_cb(int& v1, int& v2, void* context)
{
    int res = 0;
    Molecule& mol = *(Molecule*)context;
    if (mol.vertexInRing(v1) && !mol.vertexInRing(v2))
        return 1;
    else if (!mol.vertexInRing(v1) && mol.vertexInRing(v2))
        return -1;

    if ((mol.getAtomNumber(v1) == ELEM_H) && (mol.getAtomNumber(v2) != ELEM_H))
        return 1;
    else if ((mol.getAtomNumber(v1) != ELEM_H) && (mol.getAtomNumber(v2) == ELEM_H))
        return -1;

    if ((mol.getAtomNumber(v1) == ELEM_C) && (mol.getAtomNumber(v2) != ELEM_C))
        return 1;
    else if ((mol.getAtomNumber(v1) != ELEM_C) && (mol.getAtomNumber(v2) == ELEM_C))
        return -1;

    return res;
}

void MoleculeStandardizer::_clearCharges(Molecule& mol)
{
    for (auto i : mol.vertices())
    {
        mol.setAtomCharge(i, 0);
    }
}

void MoleculeStandardizer::_clearCharges(QueryMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        mol.getAtom(i).removeConstraints(QueryMolecule::ATOM_CHARGE);
        mol.resetAtom(i, QueryMolecule::Atom::und(mol.releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, 0)));
    }
}

void MoleculeStandardizer::_clearPiBonds(BaseMolecule& mol)
{
    throw Error("This option is not used for Indigo");
}

void MoleculeStandardizer::_clearHighlightColors(BaseMolecule& mol)
{
    mol.unhighlightAll();
}

void MoleculeStandardizer::_clearQueryInfo(BaseMolecule& mol)
{
    throw Error("This option is not used for Indigo");
}

void MoleculeStandardizer::_clearAtomLabels(Molecule& mol)
{
    throw Error("This option is available only for QueryMolecule object?");
}

void MoleculeStandardizer::_clearAtomLabels(QueryMolecule& mol)
{
    throw Error("This option is available only for QueryMolecule object?");
}

void MoleculeStandardizer::_clearBondLabels(Molecule& mol)
{
    throw Error("This option is available only for QueryMolecule object?");
}

void MoleculeStandardizer::_clearBondLabels(QueryMolecule& mol)
{
    throw Error("This option is available only for QueryMolecule object?");
}

void MoleculeStandardizer::_neutralizeBondedZwitterions(Molecule& mol)
{
    for (auto i : mol.vertices())
    {
        int elem = mol.getAtomNumber(i);
        if (mol.getAtomCharge(i) == 0)
            continue;

        if ((elem == ELEM_C) || (elem == ELEM_N) || (elem == ELEM_P) || (elem == ELEM_S))
        {
            const Vertex& v = mol.getVertex(i);
            for (int j : v.neighbors())
            {
                if (mol.getAtomCharge(v.neiVertex(j)) != 0)
                {
                    int c1 = mol.getAtomCharge(i);
                    int c2 = mol.getAtomCharge(v.neiVertex(j));
                    int bond = mol.getBondOrder(v.neiEdge(j));
                    if ((c1 > 0) && (c2 < 0) && (bond != BOND_TRIPLE))
                    {
                        mol.setAtomCharge(i, c1 - 1);
                        mol.setAtomCharge(v.neiVertex(j), c2 + 1);
                        if (bond == BOND_SINGLE)
                            mol.setBondOrder(v.neiEdge(j), BOND_DOUBLE);
                        else if (bond == BOND_DOUBLE)
                            mol.setBondOrder(v.neiEdge(j), BOND_TRIPLE);
                    }
                    if ((c1 < 0) && (c2 > 0) && (bond != BOND_TRIPLE))
                    {
                        mol.setAtomCharge(i, c1 + 1);
                        mol.setAtomCharge(v.neiVertex(j), c2 - 1);
                        if (bond == BOND_SINGLE)
                            mol.setBondOrder(v.neiEdge(j), BOND_DOUBLE);
                        else if (bond == BOND_DOUBLE)
                            mol.setBondOrder(v.neiEdge(j), BOND_TRIPLE);
                    }
                }
            }
        }
    }
}

void MoleculeStandardizer::_neutralizeBondedZwitterions(QueryMolecule& mol)
{
    throw Error("This option is not available for QueryMolecule object");
}

void MoleculeStandardizer::_clearUnusualValence(Molecule& mol)
{
    for (auto i : mol.vertices())
    {
        if (mol.getExplicitValence(i) > 0)
        {
            mol.setExplicitValence(i, 0);
            mol.invalidateHCounters();
        }
    }
}

void MoleculeStandardizer::_clearUnusualValence(QueryMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        mol.resetAtom(i, QueryMolecule::Atom::und(mol.releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_VALENCE, 0)));
    }
}

void MoleculeStandardizer::_clearIsotopes(Molecule& mol)
{
    for (auto i : mol.vertices())
    {
        mol.setAtomIsotope(i, 0);
    }
}

void MoleculeStandardizer::_clearIsotopes(QueryMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        mol.resetAtom(i, QueryMolecule::Atom::und(mol.releaseAtom(i), new QueryMolecule::Atom(QueryMolecule::ATOM_ISOTOPE, 0)));
    }
}

void MoleculeStandardizer::_clearDativeBonds(BaseMolecule& mol)
{
    QS_DEF(Array<int>, remove_bonds);
    remove_bonds.clear();

    for (auto i : mol.edges())
    {
        const Edge& edge = mol.getEdge(i);

        if ((mol.getBondOrder(i) == _BOND_COORDINATION) && (mol.getAtomNumber(edge.beg) != ELEM_H) && (mol.getAtomNumber(edge.end) != ELEM_H))
            remove_bonds.push(i);
    }

    if (remove_bonds.size() > 0)
        mol.removeBonds(remove_bonds);
}

void MoleculeStandardizer::_clearHydrogenBonds(BaseMolecule& mol)
{
    QS_DEF(Array<int>, remove_bonds);
    remove_bonds.clear();

    for (auto i : mol.edges())
    {
        const Edge& edge = mol.getEdge(i);

        if ((mol.getBondOrder(i) == _BOND_HYDROGEN) && ((mol.getAtomNumber(edge.beg) == ELEM_H) || (mol.getAtomNumber(edge.end) == ELEM_H)))
            remove_bonds.push(i);
    }

    if (remove_bonds.size() > 0)
        mol.removeBonds(remove_bonds);
}

void MoleculeStandardizer::_localizeMarkushRAtomsOnRings(Molecule& mol)
{
    throw Error("This option is available only for QueryMolecule object?");
}

void MoleculeStandardizer::_localizeMarkushRAtomsOnRings(QueryMolecule& mol)
{
}

void MoleculeStandardizer::_createCoordinationBonds(BaseMolecule& mol)
{
    for (auto i : mol.edges())
    {
        const Edge& edge = mol.getEdge(i);

        if ((mol.getBondOrder(i) == BOND_SINGLE) && (mol.getAtomNumber(edge.beg) != ELEM_H) && (mol.getAtomNumber(edge.end) != ELEM_H))
        {
            int non_metal_atom;
            int metal_atom;

            if (_isNonMetalAtom(mol.getAtomNumber(edge.beg)) && _isMetalAtom(mol.getAtomNumber(edge.end)))
            {
                non_metal_atom = edge.beg;
                metal_atom = edge.end;
            }
            else if (_isMetalAtom(mol.getAtomNumber(edge.beg)) && _isNonMetalAtom(mol.getAtomNumber(edge.end)))
            {
                non_metal_atom = edge.end;
                metal_atom = edge.beg;
            }
            else
                continue;

            try
            {
                mol.getAtomValence(non_metal_atom);
            }
            catch (Exception e)
            {
                if (mol.isQueryMolecule())
                    (mol.asQueryMolecule()).resetBond(i, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_ZERO));
                else
                    (mol.asMolecule()).setBondOrder(i, BOND_ZERO, false);
            }
        }
    }
}

void MoleculeStandardizer::_createHydrogenBonds(BaseMolecule& mol)
{
    QS_DEF(Array<int>, modified_atoms);
    modified_atoms.clear();

    for (auto i : mol.vertices())
    {
        if ((mol.getAtomNumber(i) == ELEM_H) && (_getNumberOfBonds(mol, i, BOND_SINGLE, false, 0) == 2))
        {
            const Vertex& v = mol.getVertex(i);
            for (auto j : v.neighbors())
            {
                bool already_modified = false;
                for (int k = 0; k < modified_atoms.size(); k++)
                {
                    if (modified_atoms[k] == v.neiVertex(j))
                    {
                        already_modified = true;
                        break;
                    }
                }
                if (!already_modified)
                {
                    modified_atoms.push(v.neiVertex(j));

                    if (mol.isQueryMolecule())
                    {
                        if (mol.findEdgeIndex(i, v.neiVertex(j)) > 0)
                            (mol.asQueryMolecule()).resetBond(mol.findEdgeIndex(i, j), new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_ZERO));
                        else
                            (mol.asQueryMolecule())
                                .resetBond(mol.findEdgeIndex(v.neiVertex(j), i), new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_ZERO));
                    }
                    else
                    {
                        if (mol.findEdgeIndex(i, v.neiVertex(j)) > 0)
                            (mol.asMolecule()).setBondOrder(mol.findEdgeIndex(i, v.neiVertex(j)), BOND_ZERO, false);
                        else
                            (mol.asMolecule()).setBondOrder(mol.findEdgeIndex(v.neiVertex(j), i), BOND_ZERO, false);
                    }
                    break;
                }
            }
        }
    }
}

int MoleculeStandardizer::_getNumberOfBonds(BaseMolecule& mol, int idx, int bond_type, bool with_element_only, int element)
{
    auto num_bonds = 0;
    const Vertex& v = mol.getVertex(idx);

    for (auto i : v.neighbors())
    {
        if (with_element_only)
        {
            if ((mol.getAtomNumber(v.neiVertex(i)) == element) && (mol.getBondOrder(v.neiEdge(i)) == bond_type))
                num_bonds++;
        }
        else
        {
            if (mol.getBondOrder(v.neiEdge(i)) == bond_type)
                num_bonds++;
        }
    }
    return num_bonds;
}

bool MoleculeStandardizer::isFragmentLinear(BaseMolecule& mol, int idx)
{
    if (!Molecule::hasCoord(mol))
        throw Error("Atoms coordinates are not defined");

    Vec3f& central_atom = mol.getAtomXyz(idx);
    const Vertex& v = mol.getVertex(idx);

    Vec3f nei_coords[2];
    int nei_count = 0;
    for (auto i : v.neighbors())
    {
        nei_coords[nei_count++] = mol.getAtomXyz(v.neiVertex(i));
    }

    Vec3f bond1, bond2;
    bond1.diff(nei_coords[0], central_atom);
    bond1.normalize();
    bond2.diff(nei_coords[1], central_atom);
    bond2.normalize();

    float angle;
    Vec3f::angle(bond1, bond2, angle);

    if (fabs(angle - M_PI) > EPSILON)
        return false;

    return true;
}

void MoleculeStandardizer::_linearizeFragment(BaseMolecule& mol, int idx)
{
    Vec3f& central_atom = mol.getAtomXyz(idx);
    const Vertex& v = mol.getVertex(idx);

    Vec3f nei_coords[2];
    int nei_count = 0;
    for (auto i : v.neighbors())
    {
        nei_coords[nei_count++] = mol.getAtomXyz(v.neiVertex(i));
    }

    central_atom.x = (nei_coords[0].x + nei_coords[1].x) / 2;
    central_atom.y = (nei_coords[0].y + nei_coords[1].y) / 2;
    central_atom.z = (nei_coords[0].z + nei_coords[1].z) / 2;

    mol.setAtomXyz(idx, central_atom);
}

bool MoleculeStandardizer::_isNonMetalAtom(int atom_number)
{
    if ((atom_number == ELEM_C) || (atom_number == ELEM_N) || (atom_number == ELEM_O) || (atom_number == ELEM_P) || (atom_number == ELEM_S) ||
        (atom_number == ELEM_Se))
        return true;

    return false;
}

bool MoleculeStandardizer::_isMetalAtom(int atom_number)
{
    if ((atom_number > 0) && (atom_number < ELEM_MAX) && !_isNonMetalAtom(atom_number) && !Element::isHalogen(atom_number) && (atom_number != ELEM_He) &&
        (atom_number != ELEM_Ne) && (atom_number != ELEM_Ar) && (atom_number != ELEM_Kr) && (atom_number != ELEM_Xe) && (atom_number != ELEM_Rn))
        return true;

    return false;
}
