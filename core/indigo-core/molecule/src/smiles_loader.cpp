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

#include <cctype>
#include <memory>
#include <regex>
#include <unordered_set>

#include "base_cpp/scanner.h"
#include "graph/cycle_basis.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"

using namespace indigo;

IMPL_ERROR(SmilesLoader, "SMILES loader");

SmilesLoader::SmilesLoader(Scanner& scanner) : _scanner(scanner)
{
    strict_aliphatic = false;
    ignorable_aam = 0;
    inside_rsmiles = false;
    ignore_closing_bond_direction_mismatch = false;
    ignore_cistrans_errors = false;
    ignore_bad_valence = false;
    ignore_no_chiral_flag = false;
    _mol = 0;
    _qmol = 0;
    _bmol = 0;
    smarts_mode = false;
    _balance = 0;
    _current_compno = 0;
    _inside_smarts_component = false;
}

SmilesLoader::~SmilesLoader()
{
    // clear pool-dependent data in this thread to avoid data races
    _atoms.clear();
}

void SmilesLoader::loadMolecule(Molecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = &mol;
    _qmol = 0;
    _has_atom_coordinates = false;
    mol.original_format = BaseMolecule::SMILES;
    _loadMolecule();
    mol.setIgnoreBadValenceFlag(ignore_bad_valence);
}

void SmilesLoader::loadQueryMolecule(QueryMolecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = 0;
    _qmol = &mol;
    mol.original_format = BaseMolecule::SMILES;
    _loadMolecule();
}

void SmilesLoader::loadSMARTS(QueryMolecule& mol)
{
    mol.clear();
    _bmol = &mol;
    _mol = 0;
    _qmol = &mol;
    smarts_mode = true;
    mol.original_format = BaseMolecule::SMARTS;
    _loadMolecule();
}

void SmilesLoader::_loadParsedMolecule()
{
    int i;

    if (_mol != 0)
    {
        for (i = 0; i < _atoms.size(); i++)
        {
            if (_atoms[i].label == 0)
                throw Error("atom without a label");
            int idx = _mol->addAtom(_atoms[i].label);
            _mol->setAtomCharge(idx, _atoms[i].charge);
            _mol->setAtomIsotope(idx, _atoms[i].isotope);
        }

        for (i = 0; i < _bonds.size(); i++)
        {
            int beg = _bonds[i].beg;
            int end = _bonds[i].end;

            if (end == -1)
                throw Error("probably pending bond %d not closed", i);
            _bonds[i].index = _mol->addBond_Silent(beg, end, _bonds[i].type);
        }
    }

    if (!smarts_mode)
        _markAromaticBonds();

    if (_mol != 0)
    {
        _addExplicitHForStereo();
        _setRadicalsAndHCounts();
    }

    if (!inside_rsmiles)
    {
        for (i = 0; i < _atoms.size(); i++)
        {
            if (_atoms[i].aam != 0)
            {
                if (_atoms[i].star_atom)
                {
                    if (_qmol != 0)
                        _qmol->resetAtom(i, new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
                    _bmol->allowRGroupOnRSite(i, _atoms[i].aam);
                }
                else
                    _bmol->reaction_atom_mapping[i] = _atoms[i].aam;
            }
            else if (_atoms[i].label == ELEM_RSITE)
            {
                if (_atoms[i].rsite_num != 0)
                    _bmol->allowRGroupOnRSite(i, _atoms[i].rsite_num);
            }
        }
    }

    if (_mol)
    {
        for (i = 0; i < _atoms.size(); i++)
        {
            if (_atoms[i].star_atom && _atoms[i].label == ELEM_PSEUDO)
            {
                _mol->setPseudoAtom(i, "*");
            }
        }
    }

    if (_qmol != 0)
        // Replace implicit H with explicit one at required stereocenter
        // or add required number of "any atom" ligands
        _addLigandsForStereo();

    _calcStereocenters();
    _calcCisTrans();

    _scanner.skipSpace();

    if (_scanner.lookNext() == '|')
    {
        _scanner.skip(1);
        _readOtherStuff();
        if (_has_atom_coordinates || _has_directions_on_rings)
        {
            std::vector<int> sensible_bond_directions;
            sensible_bond_directions.resize(_bmol->edgeCount());
            _bmol->buildFromBondsStereocenters(stereochemistry_options, sensible_bond_directions.data());
            _bmol->markBondsStereocenters();
            _bmol->markBondsAlleneStereo();
        }
    }

    // Update attachment orders for rsites
    for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
    {
        if (!_bmol->isRSite(i))
            continue;

        const Vertex& vertex = _bmol->getVertex(i);

        int j, k = 0;
        for (j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
            _bmol->setRSiteAttachmentOrder(i, vertex.neiVertex(j), k++);
    }

    if (!inside_rsmiles)
    {
        _scanner.skipSpace();
        if (!_scanner.isEOF())
            _scanner.readLine(_bmol->name, true);
    }

    if (inside_rsmiles)
    {
        _bmol->reaction_atom_mapping.clear_resize(_bmol->vertexCount() + 1);
        _bmol->reaction_atom_mapping.zerofill();
        for (i = 0; i < _atoms.size(); i++)
            _bmol->reaction_atom_mapping[i] = _atoms[i].aam;
    }
    _bmol->reaction_atom_inversion.clear_resize(_bmol->vertexCount() + 1);
    _bmol->reaction_atom_inversion.zerofill();
    _bmol->reaction_atom_exact_change.clear_resize(_bmol->vertexCount() + 1);
    _bmol->reaction_atom_exact_change.zerofill();
    _bmol->reaction_bond_reacting_center.clear_resize(_bmol->edgeCount() + 1);
    _bmol->reaction_bond_reacting_center.zerofill();

    if (ignorable_aam != 0)
    {
        ignorable_aam->clear_resize(_bmol->vertexCount());
        ignorable_aam->zerofill();
        for (i = 0; i < _atoms.size(); i++)
            ignorable_aam->at(i) = _atoms[i].ignorable_aam ? 1 : 0;
    }

    // handle the polymers (part of the CurlySMILES specification)
    for (i = 0; i < _polymer_repetitions.size(); i++)
        _handlePolymerRepetition(i);
}

void SmilesLoader::_markAromaticBonds()
{
    CycleBasis basis;
    int i;

    basis.create(*_bmol);

    // Mark all 'empty' bonds in "aromatic" rings as aromatic.
    // We use SSSR here because we do not want "empty" bonds to
    // be aromatic when they are contained in some aliphatic (SSSR) ring.
    for (i = 0; i < basis.getCyclesCount(); i++)
    {
        const Array<int>& cycle = basis.getCycle(i);
        int j;
        bool needs_modification = false;

        for (j = 0; j < cycle.size(); j++)
        {
            int idx = cycle[j];
            const Edge& edge = _bmol->getEdge(idx);

            if (!_atoms[edge.beg].aromatic || !_atoms[edge.end].aromatic)
                break;
            if (_bonds[idx].type == BOND_SINGLE || _bonds[idx].type == BOND_DOUBLE || _bonds[idx].type == BOND_TRIPLE)
                break;
            if (_qmol != 0 && !_qmol->possibleBondOrder(_bonds[idx].index, BOND_AROMATIC))
                break;
            if (_bonds[idx].type == -1)
                needs_modification = true;
        }

        if (j != cycle.size())
            continue;

        if (needs_modification)
        {
            for (j = 0; j < cycle.size(); j++)
            {
                int idx = cycle[j];
                if (_bonds[idx].type == -1)
                {
                    _bonds[idx].type = BOND_AROMATIC;
                    int bond_index = _bonds[idx].index;
                    if (_mol != 0)
                        _mol->setBondOrder_Silent(bond_index, BOND_AROMATIC);
                    if (_qmol != 0)
                        _qmol->resetBond(bond_index, QueryMolecule::Bond::und(_qmol->releaseBond(bond_index),
                                                                              new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
                }
            }
        }
    }

    for (i = 0; i < basis.getCyclesCount(); i++)
    {
        const Array<int>& cycle = basis.getCycle(i);
        int j;
        bool needs_modification = false;

        for (j = 0; j < cycle.size(); j++)
        {
            int idx = cycle[j];
            const Edge& edge = _bmol->getEdge(idx);

            if (!_atoms[edge.beg].aromatic || !_atoms[edge.end].aromatic)
            {
                needs_modification = false;
                break;
            }
            if (_bonds[idx].type == BOND_SINGLE || _bonds[idx].type == BOND_DOUBLE || _bonds[idx].type == BOND_TRIPLE)
                continue;
            if (_qmol != 0 && !_qmol->possibleBondOrder(_bonds[idx].index, BOND_AROMATIC))
                continue;
            if (_bonds[idx].type == -1)
                needs_modification = true;
        }

        if (needs_modification)
        {
            for (j = 0; j < cycle.size(); j++)
            {
                int idx = cycle[j];
                const Edge& edge = _bmol->getEdge(idx);
                if ((_bonds[idx].type == -1) && (_atoms[edge.beg].aromatic && _atoms[edge.end].aromatic))
                {
                    _bonds[idx].type = BOND_AROMATIC;
                    int bond_index = _bonds[idx].index;
                    if (_mol != 0)
                        _mol->setBondOrder_Silent(bond_index, BOND_AROMATIC);
                    if (_qmol != 0)
                        _qmol->resetBond(bond_index, QueryMolecule::Bond::und(_qmol->releaseBond(bond_index),
                                                                              new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
                }
            }
        }
    }

    // mark the rest 'empty' bonds as single
    for (i = 0; i < _bonds.size(); i++)
    {
        if (_bonds[i].type == -1)
        {
            int bond_index = _bonds[i].index;
            if (_mol != 0)
                _mol->setBondOrder_Silent(bond_index, BOND_SINGLE);
            if (_qmol != 0)
                _qmol->resetBond(bond_index,
                                 QueryMolecule::Bond::und(_qmol->releaseBond(bond_index), new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE)));
        }
    }
}

void SmilesLoader::_setRadicalsAndHCounts()
{
    int i;

    for (i = 0; i < _atoms.size(); i++)
    {
        int idx = i;

        // The SMILES specification says: Elements in the "organic subset"
        // B, C, N, O, P, S, F, Cl, Br, and I may be written without brackets
        // if the number of attached hydrogens conforms to the lowest normal
        // valence consistent with explicit bonds. We assume that there are
        // no radicals in that case.
        if (!_atoms[i].brackets)
            // We set zero radicals explicitly to properly detect errors like FClF
            // (while F[Cl]F is correct)
            _mol->setAtomRadical(idx, 0);

        if (_atoms[i].hydrogens >= 0)
            _mol->setImplicitH(idx, _atoms[i].hydrogens);
        else if (_atoms[i].brackets)    // no hydrogens in brackets?
            _mol->setImplicitH(idx, 0); // no implicit hydrogens on atom then
        else if (_atoms[i].aromatic && _mol->getAtomAromaticity(i) == ATOM_AROMATIC)
        {
            // Additional check for _mol->getAtomAromaticity(i) is required because
            // a cycle can be non-aromatic while atom letters are small
            if (_atoms[i].label == ELEM_C)
            {
                // here we are basing on the fact that
                // aromatic uncharged carbon always has a double bond
                if (_mol->getVertex(i).degree() < 3)
                    // 2-connected aromatic carbon must have 1 single bond and 1 double bond,
                    // so we have one implicit hydrogen left
                    _mol->setImplicitH(idx, 1);
                else
                    _mol->setImplicitH(idx, 0);
            }
            else
            {
                // Leave the number of hydrogens as unspecified
                // Dearomatization algorithm can find any suitable configuration
            }
        }
    }
}

void SmilesLoader::_loadMolecule()
{
    _atoms.clear();
    _bonds.clear();
    _polymer_repetitions.clear();

    _parseMolecule();
    _loadParsedMolecule();
    _validateStereoCenters();
}

SmilesLoader::_AtomDesc::_AtomDesc(Pool<List<int>::Elem>& neipool) : neighbors(neipool)
{
    label = 0;
    isotope = 0;
    charge = 0;
    hydrogens = -1;
    chirality = 0;
    aromatic = 0;
    aam = 0;
    ignorable_aam = false;
    brackets = false;
    star_atom = false;
    ends_polymer = false;
    starts_polymer = false;
    polymer_index = -1;

    parent = -1;
    rsite_num = 0;
}

SmilesLoader::_AtomDesc::~_AtomDesc()
{
}

void SmilesLoader::_AtomDesc::pending(int cycle)
{
    if (cycle < 1)
        throw Error("cycle number %d is not allowed", cycle);
    neighbors.add(-cycle);
}

void SmilesLoader::_AtomDesc::closure(int cycle, int end)
{
    int i;

    if (cycle < 1)
        throw Error("cycle number %d is not allowed", cycle);

    for (i = neighbors.begin(); i != neighbors.end(); i = neighbors.next(i))
    {
        if (neighbors.at(i) == -cycle)
        {
            neighbors.at(i) = end;
            break;
        }
    }
}

SmilesLoader::_BondDesc::_BondDesc() : beg(-1), end(-1), type(-1), dir(0), topology(0), index(-1)
{
}
