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

#include "molecule/canonical_smiles_saver.h"

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "molecule/elements.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_dearom.h"
#include "molecule/smiles_saver.h"

using namespace indigo;

IMPL_ERROR(CanonicalSmilesSaver, "canonical SMILES saver");

CanonicalSmilesSaver::CanonicalSmilesSaver(Output& output) : SmilesSaver(output), TL_CP_GET(_actual_atom_atom_mapping), TL_CP_GET(_initial_to_actual)
{
    find_invalid_stereo = true;
    ignore_invalid_hcount = false;
    ignore_hydrogens = true;
    canonize_chiralities = true;
    write_extra_info = false;
    _initial_to_actual.clear();
    _initial_to_actual.emplace(0, 0);
    _aam_counter = 0;
}

CanonicalSmilesSaver::~CanonicalSmilesSaver()
{
}

void CanonicalSmilesSaver::saveMolecule(Molecule& mol)
{
    if (mol.vertexCount() < 1)
        return;

    QS_DEF(Array<int>, ignored);
    QS_DEF(Array<int>, order);
    QS_DEF(Array<int>, ranks);

    int i;

    if (mol.sgroups.isPolimer())
        throw Error("can not canonicalize a polymer");

    // Detect hydrogens configuration if aromatic but not ambiguous
    // We can store this infromation in the original structure mol_.
    mol.restoreAromaticHydrogens();
    _mol.clone(mol, 0, 0);

    // TODO: canonicalize allenes properly
    _mol.allene_stereo.clear();

    ignored.clear_resize(_mol.vertexEnd());
    ignored.zerofill();

    for (i = _mol.vertexBegin(); i < _mol.vertexEnd(); i = _mol.vertexNext(i))
        if (_mol.convertableToImplicitHydrogen(i))
            ignored[i] = 1;

    // Try to save into ordinary smiles and find what cis-trans bonds were used
    NullOutput null_output;
    SmilesSaver saver_cistrans(null_output);
    saver_cistrans.ignore_hydrogens = true;
    saver_cistrans.saveMolecule(_mol);
    // Then reset cis-trans infromation that is not saved into SMILES
    const Array<int>& parities = saver_cistrans.getSavedCisTransParities();
    for (i = _mol.edgeBegin(); i < _mol.edgeEnd(); i = _mol.edgeNext(i))
    {
        if (_mol.cis_trans.getParity(i) != 0 && parities[i] == 0)
            _mol.cis_trans.setParity(i, 0);
    }

    MoleculeAutomorphismSearch of;

    of.detect_invalid_cistrans_bonds = find_invalid_stereo;
    of.detect_invalid_stereocenters = find_invalid_stereo;
    of.find_canonical_ordering = true;
    of.ignored_vertices = ignored.ptr();
    of.process(_mol);
    of.getCanonicalNumbering(order);

    for (i = _mol.edgeBegin(); i != _mol.edgeEnd(); i = _mol.edgeNext(i))
        if (_mol.cis_trans.getParity(i) != 0 && of.invalidCisTransBond(i))
            _mol.cis_trans.setParity(i, 0);

    for (i = _mol.vertexBegin(); i != _mol.vertexEnd(); i = _mol.vertexNext(i))
        if (_mol.stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY && of.invalidStereocenter(i))
            _mol.stereocenters.remove(i);

    ranks.clear_resize(_mol.vertexEnd());

    for (i = 0; i < order.size(); i++)
        ranks[order[i]] = i;

    vertex_ranks = ranks.ptr();

    _actual_atom_atom_mapping.clear_resize(_mol.vertexCount());
    _actual_atom_atom_mapping.zerofill();

    for (i = 0; i < order.size(); ++i)
    {
        int aam = _mol.reaction_atom_mapping[order[i]];
        if (aam)
        {
            const auto it = _initial_to_actual.find(aam);
            if (it == _initial_to_actual.end())
            {
                _initial_to_actual.emplace(aam, ++_aam_counter);
                _actual_atom_atom_mapping[order[i]] = _aam_counter;
            }
            else
            {
                _actual_atom_atom_mapping[order[i]] = it->second;
            }
        }
    }
    _mol.reaction_atom_mapping.copy(_actual_atom_atom_mapping);

    SmilesSaver::saveMolecule(_mol);
}

void CanonicalSmilesSaver::copyAAM(CanonicalSmilesSaver& saver)
{
    _actual_atom_atom_mapping.copy(saver._actual_atom_atom_mapping);
    _initial_to_actual = saver._initial_to_actual;
    _aam_counter = saver._aam_counter;
}
