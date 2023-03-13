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

#include "molecule/molecule_cip_calculator.h"

#include "base_cpp/output.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molfile_saver.h"
#include "molecule/query_molecule.h"

using namespace indigo;

bool MoleculeCIPCalculator::addCIPStereoDescriptors(BaseMolecule& mol)
{
    if (mol.have_cip)
        return false;
    QS_DEF(Array<CIPDesc>, atom_cip_desc);
    QS_DEF(Array<CIPDesc>, bond_cip_desc);
    std::unique_ptr<BaseMolecule> unfolded_h_mol;
    if (mol.isQueryMolecule())
        unfolded_h_mol = std::make_unique<QueryMolecule>();
    else
        unfolded_h_mol = std::make_unique<Molecule>();
    QS_DEF(Array<int>, markers);
    QS_DEF(Array<int>, stereo_passed);

    QS_DEF(Array<int>, ignored);

    QS_DEF(Array<EquivLigand>, equiv_ligands);

    int atom_idx, type, group, pyramid[4];

    unfolded_h_mol->clear();
    markers.clear();
    unfolded_h_mol->clone_KeepIndices(mol);
    if (!unfolded_h_mol->isQueryMolecule()) // queries are already unfolded
        unfolded_h_mol->asMolecule().unfoldHydrogens(&markers, -1, true);

    atom_cip_desc.clear_resize(unfolded_h_mol->vertexEnd());
    atom_cip_desc.zerofill();

    bond_cip_desc.clear_resize(unfolded_h_mol->edgeEnd());
    bond_cip_desc.zerofill();

    stereo_passed.clear();
    equiv_ligands.clear();

    for (auto i = mol.stereocenters.begin(); i != mol.stereocenters.end(); i = mol.stereocenters.next(i))
    {
        bool digraph_cip_used = false;

        _calcRSStereoDescriptor(mol, *unfolded_h_mol, i, atom_cip_desc, stereo_passed, false, equiv_ligands, digraph_cip_used);
        /*
           printf("Stereo descriptors for stereo center %d (1 cycle): \n", i);
           for (int k = 0; k < atom_cip_desc.size(); k++)
             printf("%d ", atom_cip_desc[k]);
           printf("\n");
        */
    }

    if (stereo_passed.size() > 0)
    {
        int nrs = _getNumberOfStereoDescritors(atom_cip_desc);
        int nrs_before = 0;
        int nrs_after = 0;
        for (;;)
        {
            nrs_before = _getNumberOfStereoDescritors(atom_cip_desc);
            bool digraph_cip_used = false;
            for (auto i = 0; i < stereo_passed.size(); i++)
            {
                mol.stereocenters.get(stereo_passed[i], atom_idx, type, group, pyramid);
                if (atom_cip_desc[atom_idx] == CIPDesc::UNKNOWN)
                    _calcRSStereoDescriptor(mol, *unfolded_h_mol, stereo_passed[i], atom_cip_desc, stereo_passed, true, equiv_ligands, digraph_cip_used);
                /*
                   printf("Stereo descriptors for stereo center %d (2 cycle): \n", i);
                   for (int k = 0; k < atom_cip_desc.size(); k++)
                     printf("%d ", atom_cip_desc[k]);
                   printf("\n");
                */
            }
            nrs_after = _getNumberOfStereoDescritors(atom_cip_desc);
            if (nrs_after == nrs_before)
                break;
        }

        if ((nrs_after - nrs) < stereo_passed.size())
        {
            ignored.clear_resize(mol.vertexEnd());
            ignored.zerofill();

            for (auto i : mol.vertices())
                if (mol.asMolecule().convertableToImplicitHydrogen(i))
                    ignored[i] = 1;

            MoleculeAutomorphismSearch as;

            as.detect_invalid_cistrans_bonds = true;
            as.detect_invalid_stereocenters = true;
            as.find_canonical_ordering = false;
            as.ignored_vertices = ignored.ptr();
            as.process(mol.asMolecule());

            for (;;)
            {
                nrs_before = _getNumberOfStereoDescritors(atom_cip_desc);
                bool digraph_cip_used = false;
                for (auto i = 0; i < stereo_passed.size(); i++)
                {
                    mol.stereocenters.get(stereo_passed[i], atom_idx, type, group, pyramid);
                    if (!as.invalidStereocenter(atom_idx) && atom_cip_desc[atom_idx] == CIPDesc::UNKNOWN)
                    {
                        _calcRSStereoDescriptor(mol, *unfolded_h_mol, stereo_passed[i], atom_cip_desc, stereo_passed, true, equiv_ligands, digraph_cip_used);
                        /*
                           printf("Stereo descriptors for stereo center %d (3 cycle): \n", i);
                           for (int k = 0; k < atom_cip_desc.size(); k++)
                             printf("%d ", atom_cip_desc[k]);
                           printf("\n");
                        */
                    }
                }
                nrs_after = _getNumberOfStereoDescritors(atom_cip_desc);
                if (nrs_after == nrs_before)
                    break;
            }
        }
    }

    for (auto i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        _calcEZStereoDescriptor(mol, *unfolded_h_mol, i, bond_cip_desc);
    }

    mol._cip_atoms.clear();
    mol._cip_bonds.clear();

    for (auto atom_idx = 0; atom_idx < atom_cip_desc.size(); ++atom_idx)
    {
        if (atom_cip_desc[atom_idx] > CIPDesc::UNKNOWN)
            mol._cip_atoms.insert(atom_idx, atom_cip_desc[atom_idx]);
    }

    for (auto bond_idx = 0; bond_idx < bond_cip_desc.size(); ++bond_idx)
    {
        if (bond_cip_desc[bond_idx] != CIPDesc::NONE)
            mol._cip_bonds.insert(bond_idx, bond_cip_desc[bond_idx]);
    }
    return mol._cip_atoms.size() || mol._cip_bonds.size();
}

int MoleculeCIPCalculator::_getNumberOfStereoDescritors(const Array<CIPDesc>& atom_cip_desc)
{
    int ndesc = 0;
    for (int i = 0; i < atom_cip_desc.size(); i++)
    {
        if (atom_cip_desc[i] == CIPDesc::s || atom_cip_desc[i] == CIPDesc::r || atom_cip_desc[i] == CIPDesc::S || atom_cip_desc[i] == CIPDesc::R)
            ndesc++;
    }
    return ndesc;
}

void MoleculeCIPCalculator::addCIPSgroups(BaseMolecule& mol)
{
    int sg_idx;

    for (int i = mol._cip_atoms.begin(); i != mol._cip_atoms.end(); i = mol._cip_atoms.next(i))
    {
        sg_idx = mol.sgroups.addSGroup(SGroup::SG_TYPE_DAT);
        DataSGroup& sgroup = (DataSGroup&)mol.sgroups.getSGroup(sg_idx);
        sgroup.atoms.push(mol._cip_atoms.key(i));

        switch (mol._cip_atoms.value(i))
        {
        case CIPDesc::R:
            sgroup.data.readString("(R)", true);
            break;
        case CIPDesc::S:
            sgroup.data.readString("(S)", true);
            break;
        case CIPDesc::r:
            sgroup.data.readString("(r)", true);
            break;
        case CIPDesc::s:
            sgroup.data.readString("(s)", true);
            break;
        }

        sgroup.name.readString("INDIGO_CIP_DESC", true);
        sgroup.display_pos.x = 0.0;
        sgroup.display_pos.y = 0.0;
        sgroup.detached = true;
        sgroup.relative = true;
    }

    for (int i = mol._cip_bonds.begin(); i != mol._cip_bonds.end(); i = mol._cip_bonds.next(i))
    {
        int bond_idx = mol._cip_bonds.key(i);
        int beg = mol.getEdge(bond_idx).beg;
        int end = mol.getEdge(bond_idx).end;

        sg_idx = mol.sgroups.addSGroup(SGroup::SG_TYPE_DAT);
        DataSGroup& sgroup = (DataSGroup&)mol.sgroups.getSGroup(sg_idx);

        sgroup.atoms.push(beg);
        sgroup.atoms.push(end);
        if (mol._cip_bonds.value(i) == CIPDesc::E)
            sgroup.data.readString("(E)", true);
        else if (mol._cip_bonds.value(i) == CIPDesc::Z)
            sgroup.data.readString("(Z)", true);

        sgroup.name.readString("INDIGO_CIP_DESC", true);
        sgroup.display_pos.x = 0.0;
        sgroup.display_pos.y = 0.0;
        sgroup.detached = true;
        sgroup.relative = true;
    }
}

void MoleculeCIPCalculator::removeCIPSgroups(BaseMolecule& mol)
{
    for (auto i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& datasgroup = (DataSGroup&)sgroup;
            if (datasgroup.name.size() > 0 && strcmp(datasgroup.name.ptr(), "INDIGO_CIP_DESC") == 0)
                mol.sgroups.remove(i);
        }
    }
}

void MoleculeCIPCalculator::convertSGroupsToCIP(BaseMolecule& mol)
{
    mol.clearCIP();
    for (auto i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& dsg = (DataSGroup&)sgroup;
            if (dsg.name.size() > 0 && std::string(dsg.name.ptr()) == "INDIGO_CIP_DESC")
            {
                auto cip_it = KSGroupToCIP.find(dsg.data.ptr());
                if (cip_it != KSGroupToCIP.end())
                {
                    switch (cip_it->second)
                    {
                    case CIPDesc::s:
                    case CIPDesc::r:
                    case CIPDesc::S:
                    case CIPDesc::R:
                        // atoms
                        for (auto atom_idx : dsg.atoms)
                            mol.setAtomCIP(atom_idx, cip_it->second);
                        break;
                    case CIPDesc::E:
                    case CIPDesc::Z:
                        // bonds
                        for (int idx = 0; idx < dsg.atoms.size() - 1; idx += 2)
                        {
                            int bond_idx = mol.findEdgeIndex(dsg.atoms[idx], dsg.atoms[idx + 1]);
                            if (bond_idx != -1)
                                mol.setBondCIP(bond_idx, cip_it->second);
                        }
                        break;
                    }
                }
                mol.sgroups.remove(i);
            }
        }
    }
}

void MoleculeCIPCalculator::_calcRSStereoDescriptor(BaseMolecule& mol, BaseMolecule& unfolded_h_mol, int idx, Array<CIPDesc>& atom_cip_desc,
                                                    Array<int>& stereo_passed, bool use_stereo, Array<EquivLigand>& equiv_ligands, bool& digraph_cip_used)
{
    Array<int> ligands;
    Array<int> used1;
    Array<int> used2;

    QS_DEF(Array<char>, st_desc);
    CIPContext context;

    int atom_idx, type, group, pyramid[4];

    int parity = 0;
    int cip_parity = 0;

    mol.stereocenters.get(idx, atom_idx, type, group, pyramid);

    if (type <= MoleculeStereocenters::ATOM_ANY)
        return;

    parity = MolfileSaver::_getStereocenterParity(mol, atom_idx);

    ligands.clear();
    used1.clear();
    used2.clear();
    ligands.copy(pyramid, 4);

    used1.push(atom_idx);
    used2.push(atom_idx);
    context.mol = &unfolded_h_mol;
    context.cip_desc = &atom_cip_desc;
    context.used1 = &used1;
    context.used2 = &used2;
    context.next_level = true;
    context.isotope_check = false;
    context.use_stereo = use_stereo;
    context.use_rule_4 = false;
    context.ref_cip1 = CIPDesc::NONE;
    context.ref_cip2 = CIPDesc::NONE;
    context.use_rule_5 = false;

    if (!digraph_cip_used)
    {
        if (_checkLigandsEquivalence(ligands, equiv_ligands, context))
        {
            if (!use_stereo)
            {
                stereo_passed.push(idx);
                atom_cip_desc[atom_idx] = CIPDesc::UNKNOWN;
            }
            else if (mol.vertexInRing(atom_idx))
            {
                atom_cip_desc[atom_idx] = _calcCIPDigraphDescriptor(unfolded_h_mol, atom_idx, ligands, equiv_ligands);
                digraph_cip_used = true;
            }
            return;
        }
        else
        {
            cipSort(ligands, &context);
        }
    }
    else if (mol.vertexInRing(atom_idx))
    {
        atom_cip_desc[atom_idx] = _calcCIPDigraphDescriptor(unfolded_h_mol, atom_idx, ligands, equiv_ligands);
        return;
    }

    if (ligands[3] == -1)
    {
        ligands[3] = mol.vertexEnd();
    }
    else
    {
        for (int k = 0; k < 4; k++)
        {
            int p = ligands[k];
            if (mol.getAtomNumber(p) == ELEM_H)
            {
                bool pure_hydrogen = (mol.getAtomIsotope(p) == 0);
                if (pure_hydrogen)
                {
                    ligands[k] = mol.vertexEnd();
                    break;
                }
            }
        }
    }

    if (MoleculeStereocenters::isPyramidMappingRigid(ligands.ptr()))
    {
        cip_parity = 1;
    }
    else
    {
        cip_parity = 2;
    }

    if (cip_parity == 1)
    {
        if (parity == 1)
        {
            if (use_stereo && context.use_rule_5)
            {
                atom_cip_desc[atom_idx] = CIPDesc::r;
            }
            else
            {
                atom_cip_desc[atom_idx] = CIPDesc::R;
            }
        }
        else
        {
            if (use_stereo && context.use_rule_5)
            {
                atom_cip_desc[atom_idx] = CIPDesc::s;
            }
            else
            {
                atom_cip_desc[atom_idx] = CIPDesc::S;
            }
        }
    }
    else
    {
        if (parity == 1)
        {
            if (use_stereo && context.use_rule_5)
            {
                atom_cip_desc[atom_idx] = CIPDesc::s;
            }
            else
            {
                atom_cip_desc[atom_idx] = CIPDesc::S;
            }
        }
        else
        {
            if (use_stereo && context.use_rule_5)
            {
                atom_cip_desc[atom_idx] = CIPDesc::r;
            }
            else
            {
                atom_cip_desc[atom_idx] = CIPDesc::R;
            }
        }
    }
    return;
}

CIPDesc MoleculeCIPCalculator::_calcCIPDigraphDescriptor(BaseMolecule& mol, int atom_idx, Array<int>& ligands, Array<EquivLigand>& equiv_ligands)
{
    QS_DEF(Molecule, digraph);
    QS_DEF(Array<int>, mapping);
    QS_DEF(Array<int>, used);
    StereocentersOptions options;
    QS_DEF(Array<int>, sensible_bond_orientations);
    CIPDesc res_cip_desc = CIPDesc::NONE;

    int i1 = -1;
    int i2 = -1;

    Molecule& source = mol.asMolecule();

    for (int k = 0; k < 3; k++)
    {
        for (int l = k + 1; l < 4; l++)
        {
            if (ligands[l] == -1)
                continue;

            for (int m = 0; m < equiv_ligands.size(); m++)
            {
                if (((ligands[k] == equiv_ligands[m][0]) && (ligands[l] == equiv_ligands[m][1])) ||
                    ((ligands[k] == equiv_ligands[m][1]) && (ligands[l] == equiv_ligands[m][0])))
                {
                    i1 = ligands[k];
                    i2 = ligands[l];
                }
            }
        }
    }
    if (i1 == -1)
        return CIPDesc::NONE;

    digraph.clear();
    used.clear();
    mapping.clear();

    int idx = digraph.addAtom(source.getAtomNumber(atom_idx));
    digraph.setAtomIsotope(idx, source.getAtomIsotope(atom_idx));
    used.push(atom_idx);
    mapping.push(atom_idx);

    _addNextLevel(source, digraph, atom_idx, idx, used, mapping);

    _calcStereocenters(source, digraph, mapping);

    QS_DEF(Array<CIPDesc>, atom_cip_desc);
    QS_DEF(Array<CIPDesc>, bond_cip_desc);
    QS_DEF(Array<int>, markers);
    QS_DEF(Array<int>, stereo_passed);

    QS_DEF(Array<int>, ignored);

    QS_DEF(Array<EquivLigand>, new_equiv_ligands);

    int new_atom_idx, type, group, pyramid[4];

    atom_cip_desc.clear_resize(digraph.vertexEnd());
    atom_cip_desc.zerofill();

    bond_cip_desc.clear_resize(digraph.edgeEnd());
    bond_cip_desc.zerofill();

    stereo_passed.clear();
    new_equiv_ligands.clear();
    bool digraph_cip_used = false;

    for (auto i = digraph.stereocenters.begin(); i != digraph.stereocenters.end(); i = digraph.stereocenters.next(i))
    {
        _calcRSStereoDescriptor(digraph, digraph, i, atom_cip_desc, stereo_passed, false, new_equiv_ligands, digraph_cip_used);
    }

    if (stereo_passed.size() > 0)
    {
        for (auto i = 0; i < stereo_passed.size(); i++)
        {
            digraph.stereocenters.get(stereo_passed[i], new_atom_idx, type, group, pyramid);
            if (atom_idx == mapping[new_atom_idx])
            {
                QS_DEF(Array<int>, used1);
                QS_DEF(Array<int>, used2);
                CIPContext context;

                Array<int> dg_ligands;
                dg_ligands.clear();
                dg_ligands.copy(pyramid, 4);

                used1.clear();
                used2.clear();
                used1.push(new_atom_idx);
                used2.push(new_atom_idx);
                context.mol = &digraph;
                context.cip_desc = &atom_cip_desc;
                context.used1 = &used1;
                context.used2 = &used2;
                context.next_level = true;
                context.isotope_check = true;
                context.use_stereo = true;
                context.use_rule_4 = false;
                context.ref_cip1 = CIPDesc::NONE;
                context.ref_cip2 = CIPDesc::NONE;
                context.use_rule_5 = false;

                int l1, l2;

                for (int k = 0; k < 4; k++)
                {
                    if (pyramid[k] != -1)
                    {
                        if (mapping[pyramid[k]] == i1)
                            l1 = pyramid[k];
                        else if (mapping[pyramid[k]] == i2)
                            l2 = pyramid[k];
                    }
                }

                cipSort(dg_ligands, &context);

                if (ligands[3] == -1)
                {
                    // Assign implicit hydrogen the highest index
                    ligands[3] = mol.vertexEnd();
                }
                else
                {
                    // Replace pure hydrogen atom with the highest value
                    for (int k = 0; k < 4; k++)
                    {
                        int p = ligands[k];
                        if (mol.getAtomNumber(p) == ELEM_H)
                        {
                            bool pure_hydrogen = (mol.getAtomIsotope(p) == 0);
                            if (pure_hydrogen)
                            {
                                ligands[k] = mol.vertexEnd();
                                break;
                            }
                        }
                    }
                }

                Array<int> sorted_ligands;
                sorted_ligands.clear();
                sorted_ligands.copy(ligands);

                for (int k = 0; k < 4; k++)
                {
                    if (dg_ligands[k] == -1)
                        sorted_ligands[k] = mol.vertexEnd();
                    else
                        sorted_ligands[k] = mapping[dg_ligands[k]];
                }

                int parity, cip_parity;

                if (MoleculeStereocenters::isPyramidMappingRigid(ligands.ptr()))
                {
                    parity = 1;
                }
                else
                {
                    parity = 2;
                }

                if (MoleculeStereocenters::isPyramidMappingRigid(sorted_ligands.ptr()))
                {
                    cip_parity = 1;
                }
                else
                {
                    cip_parity = 2;
                }

                if (cip_parity == 1)
                {
                    if (parity == 1)
                    {
                        if (context.use_rule_5)
                        {
                            res_cip_desc = CIPDesc::r;
                        }
                        else
                        {
                            res_cip_desc = CIPDesc::R;
                        }
                    }
                    else
                    {
                        if (context.use_rule_5)
                        {
                            res_cip_desc = CIPDesc::s;
                        }
                        else
                        {
                            res_cip_desc = CIPDesc::S;
                        }
                    }
                }
                else
                {
                    if (parity == 1)
                    {
                        if (context.use_rule_5)
                        {
                            res_cip_desc = CIPDesc::s;
                        }
                        else
                        {
                            res_cip_desc = CIPDesc::S;
                        }
                    }
                    else
                    {
                        if (context.use_rule_5)
                        {
                            res_cip_desc = CIPDesc::r;
                        }
                        else
                        {
                            res_cip_desc = CIPDesc::R;
                        }
                    }
                }
            }
        }
    }
    return res_cip_desc;
}

void MoleculeCIPCalculator::_addNextLevel(Molecule& source, Molecule& target, int s_idx, int t_idx, Array<int>& used, Array<int>& mapping)
{
    Array<int> next_used;

    const Vertex& v = source.getVertex(s_idx);
    int bond_stereo_atom = -1;
    int atom_h_cnt = 0;
    for (auto i : v.neighbors())
    {
        int at_idx = v.neiVertex(i);

        if (used.find(at_idx) != -1)
        {
            if ((used.find(at_idx)) != (used.size() - 2) && source.vertexInRing(s_idx))
            {
                int idx = target.addAtom(source.getAtomNumber(at_idx));
                mapping.push(at_idx);
                target.setAtomIsotope(idx, source.getAtomIsotope(at_idx));

                int b_idx = target.addBond(t_idx, idx, source.getBondOrder(v.neiEdge(i)));
                target.setBondDirection(b_idx, source.getBondDirection(v.neiEdge(i)));
            }
            continue;
        }
        else
        {
            if ((source.getBondDirection(v.neiEdge(i)) == 0) && (source.getAtomNumber(at_idx) != ELEM_H))
            {
                int idx = target.addAtom(source.getAtomNumber(at_idx));
                mapping.push(at_idx);
                target.setAtomIsotope(idx, source.getAtomIsotope(at_idx));

                int b_idx = target.addBond(t_idx, idx, source.getBondOrder(v.neiEdge(i)));
                target.setBondDirection(b_idx, source.getBondDirection(v.neiEdge(i)));

                next_used.clear();
                next_used.copy(used);
                next_used.push(at_idx);
                _addNextLevel(source, target, at_idx, idx, next_used, mapping);
            }
            else if (source.getBondDirection(v.neiEdge(i)) != 0)
            {
                bond_stereo_atom = i;
            }
            else if (source.getAtomNumber(at_idx) == ELEM_H)
            {
                atom_h_cnt++;
            }
        }
    }

    if (bond_stereo_atom != -1)
    {
        int at_idx = v.neiVertex(bond_stereo_atom);

        int idx = target.addAtom(source.getAtomNumber(at_idx));
        mapping.push(at_idx);
        target.setAtomIsotope(idx, source.getAtomIsotope(at_idx));

        int b_idx = target.addBond(t_idx, idx, source.getBondOrder(v.neiEdge(bond_stereo_atom)));
        target.setBondDirection(b_idx, source.getBondDirection(v.neiEdge(bond_stereo_atom)));

        next_used.clear();
        next_used.copy(used);
        next_used.push(at_idx);
        _addNextLevel(source, target, at_idx, idx, next_used, mapping);
    }

    if (atom_h_cnt != 0)
    {
        for (auto i : v.neighbors())
        {
            int at_idx = v.neiVertex(i);

            if (source.getAtomNumber(at_idx) == ELEM_H)
            {
                int idx = target.addAtom(source.getAtomNumber(at_idx));
                mapping.push(at_idx);
                target.setAtomIsotope(idx, source.getAtomIsotope(at_idx));

                int b_idx = target.addBond(t_idx, idx, source.getBondOrder(v.neiEdge(i)));
                target.setBondDirection(b_idx, source.getBondDirection(v.neiEdge(i)));
            }
        }
    }
}

void MoleculeCIPCalculator::_calcStereocenters(Molecule& source, Molecule& mol, Array<int>& mapping)
{
    QS_DEF(Array<int>, chirality);
    int type, group;

    chirality.clear_resize(mol.vertexEnd());
    chirality.zerofill();

    for (auto i : mol.vertices())
    {
        const Vertex& v = mol.getVertex(i);
        for (auto j : v.neighbors())
        {
            if (mol.getBondDirection2(i, v.neiVertex(j)) > 0)
            {
                chirality[i] = mol.getBondDirection2(i, v.neiVertex(j));
            }
        }
    }

    for (auto i : mol.vertices())
    {

        if ((chirality[i] != BOND_UP) && (chirality[i] != BOND_DOWN))
            continue;

        if (!mol.isPossibleStereocenter(i))
            continue;

        const Vertex& v = mol.getVertex(i);

        int pyramid[4] = {-1, -1, -1, -1};
        int counter = 0;
        int hcount = 0;
        int size = 0;

        for (auto j : v.neighbors())
        {
            int nei = v.neiVertex(j);
            if (mol.getAtomNumber(nei) != ELEM_H || mol.getAtomIsotope(nei) != 0)
                pyramid[counter++] = nei;
            else
            {
                pyramid[counter++] = -1;
                hcount++;
            }
        }

        if (hcount > 1)
            continue;

        if ((pyramid[0] == -1) || (pyramid[1] == -1) || (pyramid[2] == -1))
            continue;

        size = pyramid[3] == -1 ? 3 : 4;

        int source_pyramid[4];

        if (!source.stereocenters.exists(mapping[i]))
            continue;

        source.stereocenters.get(mapping[i], type, group, source_pyramid);

        if (source.stereocenters.isPyramidMappingRigid(source_pyramid) != mol.stereocenters.isPyramidMappingRigid(pyramid, size, mapping.ptr()))
            std::swap(pyramid[0], pyramid[1]);

        mol.addStereocenters(i, MoleculeStereocenters::ATOM_ABS, 1, pyramid);
    }
}

bool MoleculeCIPCalculator::_checkLigandsEquivalence(Array<int>& ligands, Array<EquivLigand>& equiv_ligands, CIPContext& context)
{
    int neq = 0;
    bool rule_5_used = false;

    for (int k = 0; k < 3; k++)
    {
        for (int l = k + 1; l < 4; l++)
        {
            context.ref_cip1 = CIPDesc::NONE;
            context.ref_cip2 = CIPDesc::NONE;
            context.use_rule_4 = false;
            context.use_rule_5 = false;

            if (_cip_rules_cmp(ligands[k], ligands[l], &context) == 0)
            {
                auto& equiv_pair = equiv_ligands.push();
                equiv_pair[0] = ligands[k];
                equiv_pair[1] = ligands[l];
                neq++;
            }
            else if (context.use_rule_5)
                rule_5_used = true;
        }
    }
    context.use_rule_5 = rule_5_used;

    return neq != 0;
}

bool MoleculeCIPCalculator::_isPseudoAssymCenter(BaseMolecule& mol, int idx, Array<CIPDesc>& atom_cip_desc, Array<int>& ligands,
                                                 Array<EquivLigand>& equiv_ligands)
{
    int neq = 0;
    for (int k = 0; k < 3; k++)
    {
        for (int l = k + 1; l < 4; l++)
        {
            if (ligands[l] == -1)
                continue;

            for (int m = 0; m < equiv_ligands.size(); m++)
            {
                if (((ligands[k] == equiv_ligands[m][0]) && (ligands[l] == equiv_ligands[m][1])) ||
                    ((ligands[k] == equiv_ligands[m][1]) && (ligands[l] == equiv_ligands[m][0])))
                {
                    if (((atom_cip_desc[ligands[k]] == CIPDesc::R) && (atom_cip_desc[ligands[l]] == CIPDesc::S)) ||
                        ((atom_cip_desc[ligands[k]] == CIPDesc::S) && (atom_cip_desc[ligands[l]] == CIPDesc::R)))
                    {
                        neq++;
                    }
                }
            }
        }
    }
    if (neq == 1)
        return true;

    return false;
}

void MoleculeCIPCalculator::_calcEZStereoDescriptor(BaseMolecule& mol, BaseMolecule& unfolded_h_mol, int idx, Array<CIPDesc>& bond_cip_desc)
{
    QS_DEF(Array<int>, ligands);
    QS_DEF(Array<int>, used1);
    QS_DEF(Array<int>, used2);

    QS_DEF(Array<char>, st_desc);
    CIPContext context;

    int pyramid[4];

    int cip_parity = 0;

    cip_parity = mol.cis_trans.getParity(idx);
    if (cip_parity > 0)
    {
        if (mol.getBondTopology(idx) == TOPOLOGY_RING)
        {
            if (mol.edgeSmallestRingSize(idx) <= 7)
                return;
        }

        int beg = mol.getEdge(idx).beg;
        int end = mol.getEdge(idx).end;
        memcpy(pyramid, mol.cis_trans.getSubstituents(idx), sizeof(pyramid));

        used1.clear();
        used2.clear();
        used1.push(beg);
        used2.push(beg);
        context.mol = &unfolded_h_mol;
        context.cip_desc = &bond_cip_desc;
        context.used1 = &used1;
        context.used2 = &used2;
        context.next_level = true;
        context.isotope_check = false;
        context.use_stereo = false;
        context.use_rule_4 = false;
        context.ref_cip1 = CIPDesc::NONE;
        context.ref_cip2 = CIPDesc::NONE;
        context.use_rule_5 = false;
        int cmp_res1 = _cip_rules_cmp(pyramid[0], pyramid[1], &context);

        used1.clear();
        used2.clear();
        context.cip_desc = &bond_cip_desc;
        used1.push(end);
        used2.push(end);
        context.mol = &unfolded_h_mol;
        context.used1 = &used1;
        context.used2 = &used2;
        context.next_level = true;
        context.isotope_check = false;
        context.use_stereo = false;
        context.use_rule_4 = false;
        context.ref_cip1 = CIPDesc::NONE;
        context.ref_cip2 = CIPDesc::NONE;
        context.use_rule_5 = false;
        int cmp_res2 = _cip_rules_cmp(pyramid[2], pyramid[3], &context);

        if ((cmp_res1 == 0) || (cmp_res2 == 0))
            return;

        if (cmp_res1 == cmp_res2)
        {
            if (cip_parity == 1)
            {
                bond_cip_desc[idx] = CIPDesc::Z;
            }
            else
            {
                bond_cip_desc[idx] = CIPDesc::E;
            }
        }
        else
        {
            if (cip_parity == 1)
            {
                bond_cip_desc[idx] = CIPDesc::E;
            }
            else
            {
                bond_cip_desc[idx] = CIPDesc::Z;
            }
        }
    }
    return;
}

inline void MoleculeCIPCalculator::cipSort(Array<int>& array, CIPContext* context)
{
    // FIXME: MK: This should also probably work with std::sort, but work incorrectly
    // std::sort(array.begin(), array.end(), [&context](int a, int b) { return _cip_rules_cmp(a, b, context) < 0; });
    array.qsort(MoleculeCIPCalculator::_cip_rules_cmp, context);
}

// TODO: this is waaaaay too big, need to refactor
int MoleculeCIPCalculator::_cip_rules_cmp(int i1, int i2, void* context)
{
    int res = 0;
    QS_DEF(Array<CIPDesc>, cip);
    QS_DEF(Array<int>, used1);
    QS_DEF(Array<int>, used2);

    auto* cur_context = static_cast<CIPContext*>(context);
    BaseMolecule& mol = *(BaseMolecule*)cur_context->mol;
    cip.copy(*(Array<CIPDesc>*)cur_context->cip_desc);
    used1.copy(*(Array<int>*)cur_context->used1);
    used2.copy(*(Array<int>*)cur_context->used2);

    if ((i1 == -1) && (i2 == -1))
        return 0;

    if (i1 == -1)
        return 1;

    if (i2 == -1)
        return -1;

    if (mol.getAtomNumber(i1) > mol.getAtomNumber(i2))
        return -1;
    else if (mol.getAtomNumber(i1) < mol.getAtomNumber(i2))
        return 1;

    if (cur_context->isotope_check)
    {
        int m1 = mol.getAtomIsotope(i1) == 0 ? Element::getDefaultIsotope(mol.getAtomNumber(i1)) : mol.getAtomIsotope(i1);
        int m2 = mol.getAtomIsotope(i2) == 0 ? Element::getDefaultIsotope(mol.getAtomNumber(i2)) : mol.getAtomIsotope(i2);

        if (m1 > m2)
            return -1;
        else if (m1 < m2)
            return 1;
    }

    if (cur_context->use_rule_4)
    {
        if ((cip[i2] == CIPDesc::NONE) && (cip[i1] > CIPDesc::UNKNOWN))
            return -1;
        else if ((cip[i2] > CIPDesc::UNKNOWN) && (cip[i2] < CIPDesc::S) && (cip[i1] > CIPDesc::r))
            return -1;
        else if ((cip[i1] == CIPDesc::NONE) && (cip[i2] > CIPDesc::UNKNOWN))
            return 1;
        else if ((cip[i1] > CIPDesc::UNKNOWN) && (cip[i1] < CIPDesc::S) && (cip[i2] > CIPDesc::r))
            return 1;

        if ((cip[i1] > CIPDesc::r) && (cip[i2] > CIPDesc::r))
        {
            if ((cur_context->ref_cip1 != CIPDesc::NONE) && (cur_context->ref_cip2 != CIPDesc::NONE))
            {
                if ((cip[i1] == cur_context->ref_cip1) && (cip[i2] != cur_context->ref_cip2))
                    return -1;
                else if ((cip[i1] != cur_context->ref_cip1) && (cip[i2] == cur_context->ref_cip2))
                    return 1;
            }
            else
            {
                cur_context->ref_cip1 = cip[i1];
                cur_context->ref_cip2 = cip[i2];
            }
        }
        else if ((cip[i1] == CIPDesc::r) && (cip[i2] == CIPDesc::s))
            return -1;
        else if ((cip[i1] == CIPDesc::s) && (cip[i2] == CIPDesc::r))
            return 1;
    }

    if (cur_context->use_rule_5)
    {
        if ((cip[i1] == CIPDesc::R) && (cip[i2] == CIPDesc::S))
            return -1;
        else if ((cip[i1] == CIPDesc::S) && (cip[i2] == CIPDesc::R))
            return 1;
    }

    const Vertex& v1 = mol.getVertex(i1);
    QS_DEF(Array<int>, neibs1);
    QS_DEF(Array<int>, cip_neibs1);
    neibs1.clear();
    cip_neibs1.clear();
    for (auto i : v1.neighbors())
    {
        if (used1.find(v1.neiVertex(i)) == -1)
        {
            neibs1.push(v1.neiVertex(i));
        }
    }
    if (neibs1.size() > 1)
    {
        QS_DEF(CIPContext, next_context);
        QS_DEF(Array<int>, used1_next);
        QS_DEF(Array<int>, used2_next);
        used1_next.copy(used1);
        used1_next.push(i1);
        used2_next.copy(used1_next);
        next_context.mol = &mol;
        next_context.cip_desc = &cip;
        next_context.used1 = &used1_next;
        next_context.used2 = &used2_next;
        next_context.next_level = false;
        next_context.isotope_check = cur_context->isotope_check;
        next_context.use_stereo = cur_context->use_stereo;
        next_context.use_rule_4 = cur_context->use_rule_4;
        next_context.ref_cip1 = cur_context->ref_cip1;
        next_context.ref_cip2 = cur_context->ref_cip2;
        next_context.use_rule_5 = cur_context->use_rule_5;
        cipSort(neibs1, &next_context);
    }

    cip_neibs1.copy(neibs1);

    if (mol.vertexInRing(i1))
    {
        for (auto i : v1.neighbors())
        {
            if ((used1.find(v1.neiVertex(i)) != -1) && (used1.find(v1.neiVertex(i)) != (used1.size() - 1)))
            {
                int at_idx = v1.neiVertex(i);
                int an = mol.getAtomNumber(at_idx);
                bool inserted = false;

                if (cip_neibs1.size() > 0)
                {
                    for (int j = 0; j < cip_neibs1.size(); j++)
                    {
                        if (mol.getAtomNumber(cip_neibs1[j]) < an)
                        {
                            cip_neibs1.expand(cip_neibs1.size() + 1);
                            for (auto k = cip_neibs1.size() - 1; k > j; k--)
                            {
                                cip_neibs1[k] = cip_neibs1[k - 1];
                            }
                            cip_neibs1[j] = at_idx;
                            inserted = true;
                            break;
                        }
                    }
                    if (!inserted)
                    {
                        cip_neibs1.push(at_idx);
                    }
                }
                else
                {
                    cip_neibs1.push(at_idx);
                }
            }
        }
    }

    for (auto i : v1.neighbors())
    {
        if (mol.getBondOrder(v1.neiEdge(i)) == BOND_SINGLE)
        {
        }
        else if (mol.getBondOrder(v1.neiEdge(i)) == BOND_DOUBLE)
        {
            int ins = cip_neibs1.find(v1.neiVertex(i));
            if (ins > -1)
            {
                cip_neibs1.expand(cip_neibs1.size() + 1);
                if (ins < cip_neibs1.size() - 2)
                {
                    for (auto k = cip_neibs1.size() - 1; k > ins; k--)
                    {
                        cip_neibs1[k] = cip_neibs1[k - 1];
                    }
                }
                cip_neibs1[ins + 1] = v1.neiVertex(i);
            }
            else if ((used1.find(v1.neiVertex(i)) != -1) && (used1.find(v1.neiVertex(i)) == (used1.size() - 1)))
            {
                int at_idx = v1.neiVertex(i);
                int an = mol.getAtomNumber(at_idx);
                bool inserted = false;

                if (cip_neibs1.size() > 0)
                {
                    for (int j = 0; j < cip_neibs1.size(); j++)
                    {
                        if (mol.getAtomNumber(cip_neibs1[j]) < an)
                        {
                            cip_neibs1.expand(cip_neibs1.size() + 1);
                            for (auto k = cip_neibs1.size() - 1; k > j; k--)
                            {
                                cip_neibs1[k] = cip_neibs1[k - 1];
                            }
                            cip_neibs1[j] = at_idx;
                            inserted = true;
                            break;
                        }
                    }
                    if (!inserted)
                    {
                        cip_neibs1.push(at_idx);
                    }
                }
                else
                {
                    cip_neibs1.push(at_idx);
                }
            }
        }
        else if (mol.getBondOrder(v1.neiEdge(i)) == BOND_TRIPLE)
        {
            int ins = cip_neibs1.find(v1.neiVertex(i));
            if (ins > -1)
            {
                cip_neibs1.expand(cip_neibs1.size() + 2);
                if (ins < cip_neibs1.size() - 3)
                {
                    for (auto k = cip_neibs1.size() - 1; k > ins; k--)
                    {
                        cip_neibs1[k] = cip_neibs1[k - 2];
                    }
                }
                cip_neibs1[ins + 1] = v1.neiVertex(i);
                cip_neibs1[ins + 2] = v1.neiVertex(i);
            }
            else if ((used1.find(v1.neiVertex(i)) != -1) && (used1.find(v1.neiVertex(i)) == (used1.size() - 1)))
            {
                int at_idx = v1.neiVertex(i);
                int an = mol.getAtomNumber(at_idx);
                bool inserted = false;

                if (cip_neibs1.size() > 0)
                {
                    for (int j = 0; j < cip_neibs1.size(); j++)
                    {
                        if (mol.getAtomNumber(cip_neibs1[j]) < an)
                        {
                            cip_neibs1.expand(cip_neibs1.size() + 2);
                            for (auto k = cip_neibs1.size() - 1; k > j; k--)
                            {
                                cip_neibs1[k] = cip_neibs1[k - 1];
                                cip_neibs1[k - 1] = cip_neibs1[k - 2];
                            }
                            cip_neibs1[j] = at_idx;
                            cip_neibs1[j + 1] = at_idx;
                            inserted = true;
                            break;
                        }
                    }
                    if (!inserted)
                    {
                        cip_neibs1.push(at_idx);
                        cip_neibs1.push(at_idx);
                    }
                }
                else
                {
                    cip_neibs1.push(at_idx);
                    cip_neibs1.push(at_idx);
                }
            }
        }
        else if (mol.getBondOrder(v1.neiEdge(i)) == BOND_AROMATIC)
        {
        }
    }

    const Vertex& v2 = mol.getVertex(i2);
    QS_DEF(Array<int>, neibs2);
    QS_DEF(Array<int>, cip_neibs2);
    neibs2.clear();
    cip_neibs2.clear();

    for (auto i : v2.neighbors())
    {
        if (used2.find(v2.neiVertex(i)) == -1)
        {
            neibs2.push(v2.neiVertex(i));
        }
    }
    if (neibs2.size() > 1)
    {
        QS_DEF(CIPContext, next_context);
        QS_DEF(Array<int>, used1_next);
        QS_DEF(Array<int>, used2_next);
        used1_next.copy(used2);
        used1_next.push(i2);
        used2_next.copy(used1_next);
        next_context.mol = &mol;
        next_context.cip_desc = &cip;
        next_context.used1 = &used1_next;
        next_context.used2 = &used2_next;
        next_context.next_level = false;
        next_context.isotope_check = cur_context->isotope_check;
        next_context.use_stereo = cur_context->use_stereo;
        next_context.use_rule_4 = cur_context->use_rule_4;
        next_context.ref_cip1 = cur_context->ref_cip1;
        next_context.ref_cip2 = cur_context->ref_cip2;
        next_context.use_rule_5 = cur_context->use_rule_5;
        cipSort(neibs2, &next_context);
    }

    cip_neibs2.copy(neibs2);

    if (mol.vertexInRing(i2))
    {
        for (auto i : v2.neighbors())
        {
            if ((used2.find(v2.neiVertex(i)) != -1) && (used2.find(v2.neiVertex(i)) != (used2.size() - 1)))
            {
                int at_idx = v2.neiVertex(i);
                int an = mol.getAtomNumber(at_idx);
                bool inserted = false;

                if (cip_neibs2.size() > 0)
                {
                    for (int j = 0; j < cip_neibs2.size(); j++)
                    {
                        if (mol.getAtomNumber(cip_neibs2[j]) < an)
                        {
                            cip_neibs2.expand(cip_neibs2.size() + 1);
                            for (auto k = cip_neibs2.size() - 1; k > j; k--)
                            {
                                cip_neibs2[k] = cip_neibs2[k - 1];
                            }
                            cip_neibs2[j] = at_idx;
                            inserted = true;
                            break;
                        }
                    }
                    if (!inserted)
                    {
                        cip_neibs2.push(at_idx);
                    }
                }
                else
                {
                    cip_neibs2.push(at_idx);
                }
            }
        }
    }

    for (auto i : v2.neighbors())
    {
        if (mol.getBondOrder(v2.neiEdge(i)) == BOND_SINGLE)
        {
        }
        else if (mol.getBondOrder(v2.neiEdge(i)) == BOND_DOUBLE)
        {
            int ins = cip_neibs2.find(v2.neiVertex(i));
            if (ins > -1)
            {
                cip_neibs2.expand(cip_neibs2.size() + 1);
                if (ins < cip_neibs2.size() - 2)
                {
                    for (auto k = cip_neibs2.size() - 1; k > ins; k--)
                    {
                        cip_neibs2[k] = cip_neibs2[k - 1];
                    }
                }
                cip_neibs2[ins + 1] = v2.neiVertex(i);
            }
            else if ((used2.find(v2.neiVertex(i)) != -1) && (used2.find(v2.neiVertex(i)) == (used2.size() - 1)))
            {
                int at_idx = v2.neiVertex(i);
                int an = mol.getAtomNumber(at_idx);
                bool inserted = false;

                if (cip_neibs2.size() > 0)
                {
                    for (int j = 0; j < cip_neibs2.size(); j++)
                    {
                        if (mol.getAtomNumber(cip_neibs2[j]) < an)
                        {
                            cip_neibs2.expand(cip_neibs2.size() + 1);
                            for (auto k = cip_neibs2.size() - 1; k > j; k--)
                            {
                                cip_neibs2[k] = cip_neibs2[k - 1];
                            }
                            cip_neibs2[j] = at_idx;
                            inserted = true;
                            break;
                        }
                    }
                    if (!inserted)
                    {
                        cip_neibs2.push(at_idx);
                    }
                }
                else
                {
                    cip_neibs2.push(at_idx);
                }
            }
        }
        else if (mol.getBondOrder(v2.neiEdge(i)) == BOND_TRIPLE)
        {
            int ins = cip_neibs2.find(v2.neiVertex(i));
            if (ins > -1)
            {
                cip_neibs2.expand(cip_neibs2.size() + 2);
                if (ins < cip_neibs2.size() - 3)
                {
                    for (auto k = cip_neibs2.size() - 1; k > ins; k--)
                    {
                        cip_neibs2[k] = cip_neibs2[k - 2];
                    }
                }
                cip_neibs2[ins + 1] = v2.neiVertex(i);
                cip_neibs2[ins + 2] = v2.neiVertex(i);
            }
            else if ((used2.find(v2.neiVertex(i)) != -1) && (used2.find(v2.neiVertex(i)) == (used2.size() - 1)))
            {
                int at_idx = v2.neiVertex(i);
                int an = mol.getAtomNumber(at_idx);
                bool inserted = false;

                if (cip_neibs2.size() > 0)
                {
                    for (int j = 0; j < cip_neibs2.size(); j++)
                    {
                        if (mol.getAtomNumber(cip_neibs2[j]) < an)
                        {
                            cip_neibs2.expand(cip_neibs2.size() + 2);
                            for (auto k = cip_neibs2.size() - 1; k > j; k--)
                            {
                                cip_neibs2[k] = cip_neibs2[k - 1];
                                cip_neibs2[k - 1] = cip_neibs2[k - 2];
                            }
                            cip_neibs2[j] = at_idx;
                            cip_neibs2[j + 1] = at_idx;
                            inserted = true;
                            break;
                        }
                    }
                    if (!inserted)
                    {
                        cip_neibs2.push(at_idx);
                        cip_neibs2.push(at_idx);
                    }
                }
                else
                {
                    cip_neibs2.push(at_idx);
                    cip_neibs2.push(at_idx);
                }
            }
        }
        else if (mol.getBondOrder(v2.neiEdge(i)) == BOND_AROMATIC)
        {
        }
    }

    if (cip_neibs2.size() > cip_neibs1.size())
    {
        for (auto i = 0; i < cip_neibs1.size(); i++)
        {
            res = mol.getAtomNumber(cip_neibs2[i]) - mol.getAtomNumber(cip_neibs1[i]);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }
        return 1;
    }
    else if (cip_neibs2.size() < cip_neibs1.size())
    {
        for (auto i = 0; i < cip_neibs2.size(); i++)
        {
            res = mol.getAtomNumber(cip_neibs2[i]) - mol.getAtomNumber(cip_neibs1[i]);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }
        return -1;
    }
    else if (cip_neibs1.size() > 0)
    {
        for (auto i = 0; i < cip_neibs1.size(); i++)
        {
            res = mol.getAtomNumber(cip_neibs2[i]) - mol.getAtomNumber(cip_neibs1[i]);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }

        if (cur_context->isotope_check)
        {
            for (auto i = 0; i < neibs1.size(); i++)
            {
                int m1 = mol.getAtomIsotope(neibs1[i]) == 0 ? Element::getDefaultIsotope(mol.getAtomNumber(neibs1[i])) : mol.getAtomIsotope(neibs1[i]);
                int m2 = mol.getAtomIsotope(neibs2[i]) == 0 ? Element::getDefaultIsotope(mol.getAtomNumber(neibs2[i])) : mol.getAtomIsotope(neibs2[i]);

                if (m1 > m2)
                    return -1;
                else if (m1 < m2)
                    return 1;
            }
        }

        if (cur_context->use_rule_4)
        {
            for (auto i = 0; i < neibs1.size(); i++)
            {
                if ((cip[neibs2[i]] == CIPDesc::NONE) && (cip[neibs1[i]] > CIPDesc::UNKNOWN))
                    return -1;
                else if ((cip[neibs2[i]] > CIPDesc::UNKNOWN) && (cip[neibs2[i]] < CIPDesc::S) && (cip[neibs1[i]] > CIPDesc::r))
                    return -1;
                else if ((cip[neibs1[i]] == CIPDesc::NONE) && (cip[neibs2[i]] > CIPDesc::UNKNOWN))
                    return 1;
                else if ((cip[neibs1[i]] > CIPDesc::UNKNOWN) && (cip[neibs1[i]] < CIPDesc::S) && (cip[neibs2[i]] > CIPDesc::r))
                    return 1;

                if ((cip[neibs1[i]] > CIPDesc::r) && (cip[neibs2[i]] > CIPDesc::r))
                {
                    if ((cur_context->ref_cip1 != CIPDesc::NONE) && (cur_context->ref_cip2 != CIPDesc::NONE))
                    {
                        if ((cip[neibs1[i]] == cur_context->ref_cip1) && (cip[neibs2[i]] != cur_context->ref_cip2))
                            return -1;
                        else if ((cip[neibs1[i]] != cur_context->ref_cip1) && (cip[neibs2[i]] == cur_context->ref_cip2))
                            return 1;
                    }
                    else
                    {
                        cur_context->ref_cip1 = cip[neibs1[i]];
                        cur_context->ref_cip2 = cip[neibs2[i]];
                    }
                }
                else if ((cip[neibs1[i]] == CIPDesc::r) && (cip[neibs2[i]] == CIPDesc::s))
                    return -1;
                else if ((cip[neibs1[i]] == CIPDesc::s) && (cip[neibs2[i]] == CIPDesc::r))
                    return 1;
            }
        }

        if (cur_context->use_rule_5)
        {
            for (auto i = 0; i < neibs1.size(); i++)
            {
                if ((cip[neibs1[i]] == CIPDesc::R) && (cip[neibs2[i]] == CIPDesc::S))
                    return -1;
                else if ((cip[neibs1[i]] == CIPDesc::S) && (cip[neibs2[i]] == CIPDesc::R))
                    return 1;
            }
        }

        if (!cur_context->next_level)
            return 0;

        int next_level_branches = neibs2.size() > neibs1.size() ? neibs2.size() : neibs1.size();

        if (next_level_branches > 0)
        {
            for (auto i = 0; i < next_level_branches; i++)
            {
                QS_DEF(CIPContext, next_context);
                QS_DEF(Array<int>, used1_next);
                QS_DEF(Array<int>, used2_next);
                used1_next.copy(used1);
                used1_next.push(i1);
                used2_next.copy(used2);
                used2_next.push(i2);
                next_context.mol = &mol;
                next_context.cip_desc = &cip;
                next_context.used1 = &used1_next;
                next_context.used2 = &used2_next;
                next_context.next_level = false;
                next_context.isotope_check = cur_context->isotope_check;
                next_context.use_stereo = cur_context->use_stereo;
                next_context.use_rule_4 = cur_context->use_rule_4;
                next_context.ref_cip1 = cur_context->ref_cip1;
                next_context.ref_cip2 = cur_context->ref_cip2;
                next_context.use_rule_5 = cur_context->use_rule_5;
                res = _cip_rules_cmp(neibs1[i], neibs2[i], &next_context);
                if (res > 0)
                    return 1;
                else if (res < 0)
                    return -1;
            }

            for (auto i = 0; i < next_level_branches; i++)
            {
                QS_DEF(CIPContext, next_context);
                QS_DEF(Array<int>, used1_next);
                QS_DEF(Array<int>, used2_next);
                used1_next.copy(used1);
                used1_next.push(i1);
                used2_next.copy(used2);
                used2_next.push(i2);
                next_context.mol = &mol;
                next_context.cip_desc = &cip;
                next_context.used1 = &used1_next;
                next_context.used2 = &used2_next;
                next_context.next_level = true;
                next_context.isotope_check = cur_context->isotope_check;
                next_context.use_stereo = cur_context->use_stereo;
                next_context.use_rule_4 = cur_context->use_rule_4;
                next_context.ref_cip1 = cur_context->ref_cip1;
                next_context.ref_cip2 = cur_context->ref_cip2;
                next_context.use_rule_5 = cur_context->use_rule_5;
                res = _cip_rules_cmp(neibs1[i], neibs2[i], &next_context);
                if (res > 0)
                    return 1;
                else if (res < 0)
                    return -1;
            }
        }
        else if (neibs2.size() > 0)
            return 1;
        else if (neibs1.size() > 0)
            return -1;
    }

    if (used1.size() == 1 && !cur_context->isotope_check)
    {
        int isotope_found = 0;
        for (auto i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        {
            if (mol.getAtomIsotope(i) > 0)
            {
                isotope_found = mol.getAtomIsotope(i);
            }
        }
        if (isotope_found > 0)
        {
            QS_DEF(CIPContext, next_context);
            QS_DEF(Array<int>, used1_next);
            QS_DEF(Array<int>, used2_next);
            used1_next.copy(used1);
            used2_next.copy(used2);
            next_context.mol = &mol;
            next_context.cip_desc = &cip;
            next_context.used1 = &used1_next;
            next_context.used2 = &used2_next;
            next_context.next_level = false;
            next_context.isotope_check = true;
            next_context.use_stereo = cur_context->use_stereo;
            next_context.use_rule_4 = cur_context->use_rule_4;
            next_context.ref_cip1 = cur_context->ref_cip1;
            next_context.ref_cip2 = cur_context->ref_cip2;
            next_context.use_rule_5 = cur_context->use_rule_5;
            res = _cip_rules_cmp(i1, i2, &next_context);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }

        if (isotope_found > 0)
        {
            QS_DEF(CIPContext, next_context);
            QS_DEF(Array<int>, used1_next);
            QS_DEF(Array<int>, used2_next);
            used1_next.copy(used1);
            used2_next.copy(used2);
            next_context.mol = &mol;
            next_context.cip_desc = &cip;
            next_context.used1 = &used1_next;
            next_context.used2 = &used2_next;
            next_context.next_level = true;
            next_context.isotope_check = true;
            next_context.use_stereo = cur_context->use_stereo;
            next_context.use_rule_4 = cur_context->use_rule_4;
            next_context.ref_cip1 = cur_context->ref_cip1;
            next_context.ref_cip2 = cur_context->ref_cip2;
            next_context.use_rule_5 = cur_context->use_rule_5;
            res = _cip_rules_cmp(i1, i2, &next_context);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }
    }

    if (used1.size() == 1 && cur_context->use_stereo && !cur_context->use_rule_4)
    {
        if (_getNumberOfStereoDescritors(cip) > 0)
        {
            QS_DEF(CIPContext, next_context);
            QS_DEF(Array<int>, used1_next);
            QS_DEF(Array<int>, used2_next);
            used1_next.copy(used1);
            used2_next.copy(used2);
            next_context.mol = &mol;
            next_context.cip_desc = &cip;
            next_context.used1 = &used1_next;
            next_context.used2 = &used2_next;
            next_context.next_level = false;
            next_context.isotope_check = true;
            next_context.use_stereo = cur_context->use_stereo;
            next_context.use_rule_4 = true;
            next_context.ref_cip1 = CIPDesc::NONE;
            next_context.ref_cip2 = CIPDesc::NONE;
            next_context.use_rule_5 = false;
            cur_context->use_rule_4 = true;
            res = _cip_rules_cmp(i1, i2, &next_context);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }
        if (_getNumberOfStereoDescritors(cip) > 0)
        {
            QS_DEF(CIPContext, next_context);
            QS_DEF(Array<int>, used1_next);
            QS_DEF(Array<int>, used2_next);
            used1_next.copy(used1);
            used2_next.copy(used2);
            next_context.mol = &mol;
            next_context.cip_desc = &cip;
            next_context.used1 = &used1_next;
            next_context.used2 = &used2_next;
            next_context.next_level = true;
            next_context.isotope_check = true;
            next_context.use_stereo = cur_context->use_stereo;
            next_context.use_rule_4 = true;
            next_context.ref_cip1 = CIPDesc::NONE;
            next_context.ref_cip2 = CIPDesc::NONE;
            next_context.use_rule_5 = false;
            cur_context->use_rule_4 = true;
            res = _cip_rules_cmp(i1, i2, &next_context);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }
    }

    if (used1.size() == 1 && cur_context->use_stereo && !cur_context->use_rule_5)
    {
        if (_getNumberOfStereoDescritors(cip) > 0)
        {
            QS_DEF(CIPContext, next_context);
            QS_DEF(Array<int>, used1_next);
            QS_DEF(Array<int>, used2_next);
            used1_next.copy(used1);
            used2_next.copy(used2);
            next_context.mol = &mol;
            next_context.cip_desc = &cip;
            next_context.used1 = &used1_next;
            next_context.used2 = &used2_next;
            next_context.next_level = false;
            next_context.isotope_check = true;
            next_context.use_stereo = cur_context->use_stereo;
            next_context.use_rule_4 = true;
            next_context.ref_cip1 = cip[i1];
            next_context.ref_cip2 = cip[i2];
            next_context.use_rule_5 = true;
            cur_context->use_rule_5 = true;
            res = _cip_rules_cmp(i1, i2, &next_context);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }

        if (_getNumberOfStereoDescritors(cip) > 0)
        {
            QS_DEF(CIPContext, next_context);
            QS_DEF(Array<int>, used1_next);
            QS_DEF(Array<int>, used2_next);
            used1_next.copy(used1);
            used2_next.copy(used2);
            next_context.mol = &mol;
            next_context.cip_desc = &cip;
            next_context.used1 = &used1_next;
            next_context.used2 = &used2_next;
            next_context.next_level = true;
            next_context.isotope_check = true;
            next_context.use_stereo = cur_context->use_stereo;
            next_context.use_rule_4 = true;
            next_context.ref_cip1 = cip[i1];
            next_context.ref_cip2 = cip[i2];
            next_context.use_rule_5 = true;
            cur_context->use_rule_5 = true;
            res = _cip_rules_cmp(i1, i2, &next_context);
            if (res > 0)
                return 1;
            else if (res < 0)
                return -1;
        }
    }

    return res;
}
