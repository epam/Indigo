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

#ifndef __indigo_group_pseudoatoms_expand_h__
#define __indigo_group_pseudoatoms_expand_h__

#include "molecule/molecule.h"

namespace indigo
{

    /**
     * Table of group pseudoatom labels that are expanded for V3000/molfile
     * interoperability and for proper computation of properties within Indigo
     * (e.g. molecular weight; Indigo cannot compute mass for structures that
     * contain pseudoatoms). These labels (e.g. "OH", "NH2") appear as atom
     * labels in the default monomer library of Ketcher.
     * They are not valid single-atom symbols in MOL/V3000; toolkits like RDKit
     * expect element symbols only. Expanding them to explicit atoms (O+H,
     * N+H+H) allows the molfile to be read by strict parsers and enables
     * property calculations.
     *
     * This table is the single source of truth: add new labels here as they are
     * encountered in monomer libraries (e.g. Ketcher's default library). Do not
     * duplicate this list elsewhere.
     */
    static const char* const GROUP_PSEUDOATOM_EXPAND_LABELS[] = {"OH", "NH2", nullptr};

    /**
     * Expands group pseudoatoms (see GROUP_PSEUDOATOM_EXPAND_LABELS) in the
     * given molecule into explicit atoms and bonds, in place. These pseudoatoms
     * are handled because they are defined in Ketcher's default monomer library.
     * Expansion is needed both for proper computation of properties in Indigo
     * (e.g. molecular weight, which is not defined for pseudoatoms) and for
     * V3000/molfile interoperability with downstream toolkits (e.g. RDKit).
     * Used after expandedMonomersToAtoms() so that the result is valid for
     * export and for property calculations. Distinct from
     * indigoExpandAbbreviations(), which expands SMILES-style abbreviations (Me,
     * Et, Ph, etc.) from abbreviations.xml.
     *
     * @param mol Molecule to modify (must be Molecule, not QueryMolecule).
     * @return Number of group pseudoatoms expanded.
     */
    int expandGroupPseudoatomsInMolecule(Molecule& mol);

} // namespace indigo

#endif
