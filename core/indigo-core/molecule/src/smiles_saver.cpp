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

#include "molecule/smiles_saver.h"
#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "graph/cycle_basis.h"
#include "graph/dfs_walk.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_rgroups.h"
#include "molecule/molecule_savers.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(SmilesSaver, "SMILES saver");

CP_DEF(SmilesSaver);

SmilesSaver::SmilesSaver(Output& output)
    : _output(output), CP_INIT, TL_CP_GET(_neipool), TL_CP_GET(_atoms), TL_CP_GET(_hcount), TL_CP_GET(_hcount_ignored), TL_CP_GET(_dbonds),
      TL_CP_GET(_written_atoms), TL_CP_GET(_written_atoms_inv), TL_CP_GET(_written_bonds), TL_CP_GET(_polymer_indices), TL_CP_GET(_attachment_indices),
      TL_CP_GET(_attachment_cycle_numbers), TL_CP_GET(_aromatic_bonds), TL_CP_GET(_ignored_vertices), TL_CP_GET(_complicated_cistrans), TL_CP_GET(_ban_slashes),
      TL_CP_GET(_cis_trans_parity)
{
    vertex_ranks = 0;
    ignore_hydrogens = false;
    canonize_chiralities = false;
    write_extra_info = true;
    chemaxon = true;
    _mol = nullptr;
    smarts_mode = false;
    inside_rsmiles = false;
    ignore_invalid_hcount = true;
    separate_rsites = true;
    rsite_indices_as_aam = true;
    _n_attachment_points = 0;
    _have_complicated_cistrans = false;
}

SmilesSaver::~SmilesSaver()
{
    _atoms.clear(); // to avoid data race when it is reused in another thread
}

void SmilesSaver::saveMolecule(Molecule& mol)
{
    _bmol = &mol;
    _qmol = nullptr;
    _mol = &mol;
    _saveMolecule();
}

void SmilesSaver::saveQueryMolecule(QueryMolecule& mol)
{
    _bmol = &mol;
    _qmol = &mol;
    _mol = nullptr;
    _saveMolecule();
}

void SmilesSaver::_saveMolecule()
{
    int i, j, k;

    _ignored_vertices.clear_resize(_bmol->vertexEnd());
    _ignored_vertices.zerofill();
    if (ignore_hydrogens)
    {
        if (_qmol != 0)
            throw Error("ignore_hydrogens does not make sense for query molecules");

        for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
            if (_mol->asMolecule().convertableToImplicitHydrogen(i))
                _ignored_vertices[i] = 1;
    }

    _checkRGroupsAndAttachmentPoints();
    _checkSRU();

    _touched_cistransbonds = 0;
    _complicated_cistrans.clear_resize(_bmol->edgeEnd());
    _complicated_cistrans.zerofill();
    _ban_slashes.clear_resize(_bmol->edgeEnd());
    _ban_slashes.zerofill();
    _cis_trans_parity.clear_resize(_bmol->edgeEnd());
    _cis_trans_parity.zerofill();
    _markCisTrans();

    _atoms.clear();
    while (_atoms.size() < _bmol->vertexEnd())
        _atoms.push(_neipool);

    _written_atoms.clear();
    _written_bonds.clear();
    _written_components = 0;

    _aromatic_bonds.clear();

    _hcount.clear_resize(_bmol->vertexEnd());
    _hcount_ignored.clear_resize(_bmol->vertexEnd());

    for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
    {
        _hcount[i] = 0;
        if (_mol != 0)
        {
            if (!_mol->isPseudoAtom(i) && !_mol->isRSite(i))
                _hcount[i] = _mol->getImplicitH_NoThrow(i, -1);
        }
        else
        {
            int atom_number = _bmol->getAtomNumber(i);
            int atom_charge = _bmol->getAtomCharge(i);
            _hcount[i] = MoleculeSavers::getHCount(*_bmol, i, atom_number, atom_charge);
        }
        _hcount_ignored[i] = 0;

        const Vertex& vertex = _bmol->getVertex(i);

        if (ignore_hydrogens)
        {
            if (_hcount[i] >= 0)
            {
                for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                {
                    int idx = vertex.neiVertex(j);
                    if (_bmol->getAtomNumber(idx) == ELEM_H && _bmol->getAtomIsotope(idx) == 0)
                        if (_ignored_vertices[idx])
                            _hcount_ignored[i]++;
                }
                _hcount[i] += _hcount_ignored[i];
            }
        }

        if (_bmol->getAtomAromaticity(i) == ATOM_AROMATIC)
        {
            _atoms[i].aromatic = true;
            // From the SMILES specification:
            // Please note that only atoms on the following list
            // can be considered aromatic: C, N, O, P, S, As, Se, and * (wildcard).
            static int allowed_lowercase[] = {ELEM_C, ELEM_N, ELEM_O, ELEM_P, ELEM_S, ELEM_Se, ELEM_As};
            if (_bmol->atomNumberBelongs(i, allowed_lowercase, NELEM(allowed_lowercase)))
                _atoms[i].lowercase = true;
        }
    }

    DfsWalk walk(*_bmol);

    walk.ignored_vertices = _ignored_vertices.ptr();
    walk.vertex_ranks = vertex_ranks;
    if (_bmol->sgroups.isPolimer())
        walk.vertex_classes = _polymer_indices.ptr();

    if (separate_rsites)
    {
        for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
        {
            if (_bmol->isRSite(i))
                // We break the DFS walk when going through R-sites. For details, see
                // http://blueobelisk.shapado.com/questions/how-r-group-atoms-should-be-represented-in-smiles
                walk.mustBeRootVertex(i);
        }
    }

    walk.walk();

    const Array<DfsWalk::SeqElem>& v_seq = walk.getSequence();
    Array<int> v_to_comp_group;
    v_to_comp_group.resize(v_seq.size());
    v_to_comp_group.fffill();

    if (_qmol != nullptr && smarts_mode)
    {
        if (v_seq.size() < 1)
            return; // No atoms to save
        std::set<int> components;
        int cur_component = -1;
        for (int i = 0; i < v_seq.size(); ++i && _qmol->components.size() > 0)
        {
            // In v_seq each fragment started with vertex which parent == -1
            // In SMARTS some fragments could be grouped (component-level grouping)
            // In QueryMolecule group number stored in "".components" member. GroupId == 0 means no group defined.
            // Each fragment - connected graph, so all vertexes should belong to one group.
            // All group fragments should go one by one - in SMARTS its inside "()".
            if (v_seq[i].parent_vertex < 0) // New Fragment
            {
                int new_component = 0;
                if (_qmol->components.size() > v_seq[i].idx)
                {
                    new_component = _qmol->components[v_seq[i].idx];
                    // if component defined for new fragment(id>0) and its different from previous and seen before
                    if (new_component > 0 && new_component != cur_component && components.count(new_component))
                    {
                        // According to the DfsWalk code, the groups components should be neighbors.
                        // If will be found case when it wrong - add code to rearrange fragments
                        throw Error("SMARTS fragments need to reaarange.");
                    }
                    components.emplace(new_component);
                }
                cur_component = new_component;
            }
            else
            {
                if (_qmol->components.size() > v_seq[i].idx && cur_component != _qmol->components[v_seq[i].idx])
                {
                    // Fragment contains atoms from different components - something went wrong
                    throw Error("Fragment contains atoms from different components.");
                }
            }
            v_to_comp_group[i] = cur_component;
        }
    }

    // fill up neighbor lists for the stereocenters calculation
    for (i = 0; i < v_seq.size(); i++)
    {
        int v_idx = v_seq[i].idx;
        int e_idx = v_seq[i].parent_edge;
        int v_prev_idx = v_seq[i].parent_vertex;

        _Atom& atom = _atoms[v_idx];

        if (e_idx >= 0)
        {
            if (walk.isClosure(e_idx))
            {
                int k;
                for (k = atom.neighbors.begin(); k != atom.neighbors.end(); k = atom.neighbors.next(k))
                {
                    if (atom.neighbors[k] == -1)
                    {
                        atom.neighbors[k] = v_prev_idx;
                        break;
                    }
                }
                if (k == atom.neighbors.end())
                    throw Error("internal: can not put closing bond to its place");
            }
            else
            {
                atom.neighbors.add(v_prev_idx);
                atom.parent = v_prev_idx;
            }
            _atoms[v_prev_idx].neighbors.add(v_idx);
        }

        if (e_idx < 0 || !walk.isClosure(e_idx))
        {
            int openings = walk.numOpenings(v_idx);

            for (j = 0; j < openings; j++)
                atom.neighbors.add(-1);
        }

        // also, detect polymer borders
        if (_polymer_indices[v_idx] >= 0 && (v_prev_idx == -1 || _polymer_indices[v_prev_idx] != _polymer_indices[v_idx]))
            atom.starts_polymer = true;

        if (v_prev_idx >= 0 && _polymer_indices[v_prev_idx] >= 0 && _polymer_indices[v_prev_idx] != _polymer_indices[v_idx])
            _atoms[v_prev_idx].ends_polymer = true;
    }

    _written_atoms_inv.clear_resize(_bmol->vertexEnd());
    _written_atoms_inv.fffill();

    for (i = 0; i < v_seq.size(); i++)
    {
        int v_idx = v_seq[i].idx;
        int e_idx = v_seq[i].parent_edge;

        if (e_idx == -1 || !walk.isClosure(e_idx))
        {
            _written_atoms.push(v_idx);
            _written_atoms_inv[v_idx] = _written_atoms.size() - 1;
        }

        if (e_idx != -1)
            _written_bonds.push(e_idx);
    }

    // detect chiral configurations
    MoleculeStereocenters& stereocenters = _bmol->stereocenters;

    for (i = stereocenters.begin(); i != stereocenters.end(); i = stereocenters.next(i))
    {
        int atom_idx, type, group, pyramid[4];

        stereocenters.get(i, atom_idx, type, group, pyramid);

        if (type < MoleculeStereocenters::ATOM_AND || !stereocenters.isTetrahydral(atom_idx))
            continue;

        int implicit_h_idx = -1;

        if (pyramid[3] == -1)
            implicit_h_idx = 3;
        else
            for (j = 0; j < 4; j++)
                if (_ignored_vertices[pyramid[j]])
                {
                    implicit_h_idx = j;
                    break;
                }

        int pyramid_mapping[4];
        int counter = 0;

        _Atom& atom = _atoms[atom_idx];

        if (atom.parent != -1)
            for (k = 0; k < 4; k++)
                if (pyramid[k] == atom.parent)
                {
                    pyramid_mapping[counter++] = k;
                    break;
                }

        if (implicit_h_idx != -1)
            pyramid_mapping[counter++] = implicit_h_idx;

        for (j = atom.neighbors.begin(); j != atom.neighbors.end(); j = atom.neighbors.next(j))
        {
            if (atom.neighbors[j] == atom.parent)
                continue;

            for (k = 0; k < 4; k++)
                if (atom.neighbors[j] == pyramid[k])
                {
                    if (counter >= 4)
                        throw Error("internal: pyramid overflow");
                    pyramid_mapping[counter++] = k;
                    break;
                }
        }

        if (counter == 4)
        {
            // move the 'from' atom to the end
            counter = pyramid_mapping[0];
            pyramid_mapping[0] = pyramid_mapping[1];
            pyramid_mapping[1] = pyramid_mapping[2];
            pyramid_mapping[2] = pyramid_mapping[3];
            pyramid_mapping[3] = counter;
        }
        else if (counter != 3)
            throw Error("cannot calculate chirality");

        if (MoleculeStereocenters::isPyramidMappingRigid(pyramid_mapping))
            _atoms[atom_idx].chirality = 1;
        else
            _atoms[atom_idx].chirality = 2;
    }

    MoleculeAlleneStereo& allene_stereo = _bmol->allene_stereo;

    for (i = allene_stereo.begin(); i != allene_stereo.end(); i = allene_stereo.next(i))
    {
        int atom_idx, left, right, parity, subst[4], subst_map[4] = {-1, -1, -1, -1};

        allene_stereo.get(i, atom_idx, left, right, subst, parity);

        for (j = 0; j < 4; j++)
            if (subst[j] >= 0)
                subst_map[j] = _written_atoms_inv[subst[j]];

        // Daylight doc says: Hydrogens attached to substituted allene-like atoms
        // are taken to be immediately following that atom
        if (subst_map[0] < 0)
            subst_map[0] = _written_atoms_inv[left];
        else if (subst_map[1] < 0)
            subst_map[1] = _written_atoms_inv[left];

        if (subst_map[2] < 0)
            subst_map[2] = _written_atoms_inv[right];
        else if (subst_map[3] < 0)
            subst_map[3] = _written_atoms_inv[right];

        if (subst_map[0] < 0 || subst_map[1] < 0 || subst_map[2] < 0 || subst_map[3] < 0)
            throw Error("internal: allene subsituent not mapped");

        if (subst_map[1] < subst_map[0])
        {
            std::swap(subst_map[0], subst_map[1]);
            parity = 3 - parity;
        }
        if (subst_map[3] < subst_map[2])
        {
            std::swap(subst_map[2], subst_map[3]);
            parity = 3 - parity;
        }

        _atoms[atom_idx].chirality = 3 - parity;
    }

    if (canonize_chiralities)
    {
        int i, j;
        QS_DEF(Array<int>, marked);
        QS_DEF(Array<int>, ids);
        const MoleculeStereocenters& stereocenters = _bmol->stereocenters;

        marked.clear_resize(_bmol->vertexEnd());
        marked.zerofill();

        for (i = 0; i < _written_atoms.size(); i++)
        {
            if (marked[i])
                continue;

            int idx = _written_atoms[i];

            if (_atoms[idx].chirality == 0 || !stereocenters.isTetrahydral(idx))
                continue;

            int type = stereocenters.getType(idx);

            if (type != MoleculeStereocenters::ATOM_AND && type != MoleculeStereocenters::ATOM_OR)
                continue;

            ids.clear();
            ids.push(idx);

            int group = stereocenters.getGroup(idx);

            for (j = i + 1; j < _written_atoms.size(); j++)
            {
                if (marked[j])
                    continue;

                int idx2 = _written_atoms[j];

                if (_atoms[idx2].chirality == 0)
                    continue;

                int type2 = stereocenters.getType(idx2);
                int group2 = stereocenters.getGroup(idx2);

                if (type2 == type && group2 == group)
                {
                    ids.push(idx2);
                    marked[j] = 1;
                }
            }

            if (_atoms[ids[0]].chirality == 1)
                for (j = 0; j < ids.size(); j++)
                    _atoms[ids[j]].chirality = 3 - _atoms[ids[j]].chirality;
        }
    }

    // write the SMILES itself

    // cycle_numbers[i] == -1 means that the number is available
    // cycle_numbers[i] == n means that the number is used by vertex n
    QS_DEF(Array<int>, cycle_numbers);

    int rsites_closures_starting_num = 91;
    int rbonds = _countRBonds() + _n_attachment_points;

    if (rbonds > 9)
        rsites_closures_starting_num = 99 - rbonds;

    cycle_numbers.clear();
    cycle_numbers.push(0); // never used

    bool first_component = true;

    for (i = 0; i < v_seq.size(); i++)
    {
        int v_idx = v_seq[i].idx;
        int e_idx = v_seq[i].parent_edge;
        int v_prev_idx = v_seq[i].parent_vertex;
        bool write_atom = true;

        if (v_prev_idx >= 0)
        {
            int branches = walk.numBranches(v_prev_idx);

            if (branches > 1)
                if (_atoms[v_prev_idx].branch_cnt > 0 && _atoms[v_prev_idx].paren_written)
                    _output.writeChar(')');
            /*
             * Fix IND-673 unused if-statement
             */
            //         if (v_prev_idx >= 0)
            //         {

            if (branches > 1)
                if (_atoms[v_prev_idx].branch_cnt < branches - 1)
                {
                    if (walk.isClosure(e_idx))
                        _atoms[v_prev_idx].paren_written = false;
                    else
                    {
                        _output.writeChar('(');
                        _atoms[v_prev_idx].paren_written = true;
                    }
                }

            _atoms[v_prev_idx].branch_cnt++;

            if (_atoms[v_prev_idx].branch_cnt > branches)
                throw Error("unexpected branch");
            /*
             * Fix IND-673 unused if-statement
             */
            //         }

            const Edge& edge = _bmol->getEdge(e_idx);
            bool bond_written = true;

            int dir = 0;
            int bond_order = _bmol->getBondOrder(e_idx);

            // SMARTS and KET loaders store directions in
            if (!smarts_mode || _qmol == nullptr) // || (original_format != ket && original_format != smarts))
                if (bond_order == BOND_SINGLE)
                    dir = _calcBondDirection(e_idx, v_prev_idx);

            if ((dir == 1 && v_idx == edge.end) || (dir == 2 && v_idx == edge.beg))
                _output.writeChar('/');
            else if ((dir == 2 && v_idx == edge.end) || (dir == 1 && v_idx == edge.beg))
                _output.writeChar('\\');
            else if (smarts_mode && _qmol != 0)
                QueryMolecule::writeSmartsBond(_output, &_qmol->getBond(e_idx), false);
            else if (bond_order == BOND_DOUBLE)
                _output.writeChar('=');
            else if (bond_order == BOND_TRIPLE)
                _output.writeChar('#');
            else if (bond_order == BOND_AROMATIC && _shouldWriteAromaticBond(e_idx))
                _output.writeChar(':');
            else if (bond_order == BOND_SINGLE && _atoms[edge.beg].aromatic && _atoms[edge.end].aromatic)
                _output.writeChar('-');
            else
                bond_written = false;

            if (walk.isClosure(e_idx))
            {
                for (j = 1; j < cycle_numbers.size(); j++)
                    if (cycle_numbers[j] == v_idx)
                        break;

                if (j == cycle_numbers.size())
                    throw Error("cycle number not found");

                _writeCycleNumber(j);

                cycle_numbers[j] = -1;
                write_atom = false;
            }
        }
        else
        {
            if (!first_component)
            {
                // group == 0 means no group set.
                int prev_group = v_to_comp_group[i - 1];
                int new_group = v_to_comp_group[i];
                bool different_groups = new_group != prev_group;
                if (smarts_mode && prev_group && different_groups) // if component group ended
                    _output.writeChar(')');

                _output.writeChar('.');

                if (smarts_mode && new_group && different_groups) // if new group started
                    _output.writeChar('(');
            }
            else
            {
                if (smarts_mode && v_to_comp_group[i] > 0) // component level grouping set for this fragment
                    _output.writeChar('(');
                first_component = false;
            }
            _written_components++;
        }
        if (write_atom)
        {
            if (!smarts_mode)
                _writeAtom(v_idx, _atoms[v_idx].aromatic, _atoms[v_idx].lowercase, _atoms[v_idx].chirality);
            else if (_qmol != 0)
            {
                int aam = _bmol->reaction_atom_mapping[v_idx];
                QueryMolecule::writeSmartsAtom(_output, &_qmol->getAtom(v_idx), aam, _atoms[v_idx].chirality, 0, false, false, _qmol->original_format);
            }
            else
                throw Error("SMARTS format available for query only!");

            QS_DEF(Array<int>, closing);

            walk.getNeighborsClosing(v_idx, closing);

            for (int ap = 1; ap <= _bmol->attachmentPointCount(); ap++)
            {
                int idx = 0, atom_idx;

                for (idx = 0; (atom_idx = _bmol->getAttachmentPoint(ap, idx)) != -1; idx++)
                    if (atom_idx == v_idx)
                    {
                        // Here is the problem: if we have an attachment point, the resulting
                        // SMILES is supposed to contain an extra atom not present in the given
                        // molecule. For example, chlorine with an attachment point will
                        // become Cl%91.[*:1]%91 |;_AP1| (two atoms).
                        // We can not modify the given molecule, but we want the closure to
                        // be here. To achieve that, we add a link to an imagimary atom with
                        // incredibly big number.
                        closing.push(10000 + ap);
                    }
            }

            for (j = 0; j < closing.size(); j++)
            {
                bool ap = (closing[j] >= 10000);
                bool rsite = !ap && (separate_rsites && _bmol->isRSite(closing[j]));

                if (ap || rsite)
                {
                    cycle_numbers.expandFill(rsites_closures_starting_num + 1, -1);
                    for (k = rsites_closures_starting_num; k < cycle_numbers.size(); k++)
                        if (cycle_numbers[k] == -1)
                            break;
                }
                else
                {
                    for (k = 1; k < cycle_numbers.size(); k++)
                        if (cycle_numbers[k] == -1)
                            break;
                }

                if (ap)
                {
                    _attachment_indices.push(closing[j] - 10000);
                    _attachment_cycle_numbers.push(k);
                }

                if (k == cycle_numbers.size())
                    cycle_numbers.push(v_idx);
                else
                    cycle_numbers[k] = v_idx;

                _writeCycleNumber(k);
            }

            if (_atoms[v_idx].starts_polymer)
                _output.writeString("{-}");
            if (_atoms[v_idx].ends_polymer)
                _output.writeString("{+n}");
        }
    }
    if (smarts_mode && v_to_comp_group[i - 1] > 0) // if group set for last fragment - add finish )
        _output.writeChar(')');

    if (write_extra_info && chemaxon && !smarts_mode) // no extended block in SMARTS
    {
        // Before we write the |...| block (ChemAxon's Extended SMILES),
        // we must clean up the mess we did with the attachment points
        // (see big comment above). That is, we append separated atoms,
        // not present in the original molecule, to the end, and connect
        // them with the "cycle" closure to the real atoms that are the
        // attachment points.
        for (i = 0; i < _attachment_indices.size(); i++)
        {
            _output.printf(".[*:%d]", _attachment_indices[i]);
            _writeCycleNumber(_attachment_cycle_numbers[i]);
        }

        _comma = false;
        _writeRingCisTrans();
        _writeStereogroups();
        _writeRadicals();
        _writePseudoAtoms();
        _writeHighlighting();
        _writeRGroups();
        _writeSGroups();
        _writeRingBonds();
        _writeUnsaturated();
        _writeSubstitutionCounts();
        if (_bmol->hasAtropoStereoBonds())
            _writeWedges();

        if (_comma)
            _output.writeChar('|');
    }
}

bool SmilesSaver::_shouldWriteAromaticBond(int e_idx)
{
    const Edge& edge = _bmol->getEdge(e_idx);

    if (_mol == 0)
        return true;

    if (!_atoms[edge.beg].lowercase || !_atoms[edge.end].lowercase)
        return true;

    if (_bmol->getBondTopology(e_idx) != TOPOLOGY_RING)
        return true;

    return false;
}

void SmilesSaver::_writeCycleNumber(int n) const
{
    if (n > 0 && n < 10)
        _output.printf("%d", n);
    else if (n >= 10 && n < 100)
        _output.printf("%%%2d", n);
    else
        throw Error("bad cycle number: %d", n);
}

void SmilesSaver::_writeAtom(int idx, bool aromatic, bool lowercase, int chirality) const
{
    int i;
    bool need_brackets = false;
    int hydro = -1;
    int aam = 0;

    if (_bmol->isRSite(idx))
    {
        if (rsite_indices_as_aam && _bmol->getRSiteBits(idx) != 0)
        {
            QS_DEF(Array<int>, allowed_rgroups);
            _bmol->getAllowedRGroups(idx, allowed_rgroups);
            _output.printf("[*:%d]", allowed_rgroups[0]);
        }
        else
            _output.printf("[*]");

        return;
    }

    int atom_number = _bmol->getAtomNumber(idx);
    int charge = _bmol->getAtomCharge(idx);
    int isotope = _bmol->getAtomIsotope(idx);

    if (charge == CHARGE_UNKNOWN)
        charge = 0;

    if (_bmol->isPseudoAtom(idx)) // pseudo-atom
    {
        if ((chirality == 0) && (charge == 0))
            _output.printf("*");
        else
        {
            _output.printf("[*");
            _writeChirality(chirality);
            _writeCharge(charge);
            _output.printf("]");
        }

        return;
    }

    if (atom_number < 1)
    {
        if (_qmol != 0)
        {
            // Check special atom
            if (QueryMolecule::queryAtomIsSpecial(*_qmol, idx))
            {
                _output.printf("*");
                return;
            }

            // Check for !H meaning any atom in SMILES
            int value;
            if (_qmol->getAtom(idx).sureValueInv(QueryMolecule::ATOM_NUMBER, value))
            {
                if (value == ELEM_H)
                {
                    if ((chirality == 0) && (charge == 0))
                        _output.printf("*");
                    else
                    {
                        _output.printf("[*");
                        _writeChirality(chirality);
                        _writeCharge(charge);
                        _output.printf("]");
                    }
                    return;
                }
            }
            // Check atom list

            QS_DEF(Array<int>, list);

            int query_atom_type;
            if ((query_atom_type = QueryMolecule::parseQueryAtom(*_qmol, idx, list)) != -1)
            {
                if (list.size() > 0)
                    throw Error("atom list can be used only with smarts_mode");
            }
        }

        throw Error("undefined atom number");
    }

    if (inside_rsmiles)
        aam = _bmol->reaction_atom_mapping[idx];

    if (!QueryMolecule::isOrganicSubset(atom_number))
        need_brackets = true;

    if (chirality > 0 || charge != 0 || isotope > 0 || aam > 0)
        need_brackets = true;

    // Ignored hydrogens will be converted to implicit hydrogens.
    // So number of ignored hydrogens should be passed into
    // shouldWriteHCount to save correctly [H]S([H])([H])C
    // as C[SH3] when hydrogens are ignored (as in canonical smiles)
    // instead of getting CS.
    if (_mol != 0)
    {
        if (Molecule::shouldWriteHCountEx(*_mol, idx, _hcount_ignored[idx]))
        {
            hydro = _hcount[idx];
            if (hydro < 0 && !ignore_invalid_hcount && need_brackets)
            {
                // This function will throw better error message with a description
                _mol->getImplicitH(idx);
                // If not error was thrown then throw it explicitly
                throw Error("unsure hydrogen count on atom #%d", idx);
            }
        }
    }
    else if (_qmol != 0)
    {
        // For query molecules write hydrogens if is was specified
        hydro = _hcount[idx];
    }
    if (_qmol != 0)
        _qmol->getAtom(idx).sureValue(QueryMolecule::ATOM_TOTAL_H, hydro);

    if (hydro >= 0)
        need_brackets = true;

    if (need_brackets)
    {
        // Check sure number of hydrogens only for the ordinary molecule
        if (hydro == -1 && _mol != 0)
        {
            hydro = _hcount[idx];

            if (hydro < 0 && !ignore_invalid_hcount)
                throw Error("unsure hydrogen count on atom #%d", idx);
        }
        if (chirality > 0 && hydro > 1)
        {
            if (charge != 0 || isotope > 0 || aam > 0)
                _output.writeChar('[');
            else
                need_brackets = false;
        }
        else
            _output.writeChar('[');
    }

    if (isotope > 0)
        _output.printf("%d", isotope);

    const char* elem = Element::toString(atom_number);

    if (lowercase)
    {
        for (i = 0; i < (int)strlen(elem); i++)
            _output.printf("%c", tolower(elem[i]));
    }
    else
        _output.printf("%s", elem);

    if (!need_brackets)
        return;

    if (hydro < 2)
        _writeChirality(chirality);

    if (hydro > 1)
        _output.printf("H%d", hydro);
    else if (hydro == 1)
        _output.printf("H");

    _writeCharge(charge);

    if (aam > 0)
        _output.printf(":%d", aam);

    if (need_brackets)
        _output.writeChar(']');
}

void SmilesSaver::_writeChirality(int chirality) const
{
    if (chirality > 0)
    {
        if (chirality == 1)
            _output.printf("@");
        else // chirality == 2
            _output.printf("@@");
    }
}

void SmilesSaver::_writeCharge(int charge) const
{
    if (charge > 1)
        _output.printf("+%d", charge);
    else if (charge < -1)
        _output.printf("-%d", -charge);
    else if (charge == 1)
        _output.printf("+");
    else if (charge == -1)
        _output.printf("-");
}

void SmilesSaver::_banSlashes()
{
    QS_DEF(Array<int>, slashes);
    BaseMolecule& mol = *_bmol;
    int i, j;

    slashes.clear_resize(mol.edgeEnd());
    slashes.zerofill();

    // mark bonds that are are about to be written as slashes
    for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        // those are only chain cis-trans bonds
        if (_cis_trans_parity[i] != 0)
        {
            const Vertex& beg = mol.getVertex(mol.getEdge(i).beg);
            const Vertex& end = mol.getVertex(mol.getEdge(i).end);

            if (mol.getEdgeTopology(i) == TOPOLOGY_CHAIN)
            {
                for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
                    if (mol.getBondOrder(beg.neiEdge(j)) == BOND_SINGLE)
                        slashes[beg.neiEdge(j)] = 1;

                for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
                    if (mol.getBondOrder(end.neiEdge(j)) == BOND_SINGLE)
                        slashes[end.neiEdge(j)] = 1;
            }
            else
            {
                // Do not write slashes in rings till it is not fixed
                for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
                    if (mol.getBondOrder(beg.neiEdge(j)) == BOND_SINGLE)
                        _ban_slashes[beg.neiEdge(j)] = 1;

                for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
                    if (mol.getBondOrder(end.neiEdge(j)) == BOND_SINGLE)
                        _ban_slashes[end.neiEdge(j)] = 1;
            }
        }
    }

    // find if any "potentially but not actively cis-trans" or ring cis-trans bond will be affected
    // by the slashes
    for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        if (!mol.cis_trans.isGeomStereoBond(mol, i, 0, false))
            continue;

        if (mol.getEdgeTopology(i) == TOPOLOGY_RING || _cis_trans_parity[i] == 0)
        {
            const Vertex& beg = mol.getVertex(mol.getEdge(i).beg);
            const Vertex& end = mol.getVertex(mol.getEdge(i).end);
            bool slash_beg = false;
            bool slash_end = false;

            for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
                if (slashes[beg.neiEdge(j)])
                {
                    slash_beg = true;
                    break;
                }

            for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
                if (slashes[end.neiEdge(j)])
                {
                    slash_end = true;
                    break;
                }

            if (!slash_beg || !slash_end)
                // the bond will not be affected by slashes (at lest, not both sides of it)
                continue;

            for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
                if (slashes[beg.neiEdge(j)])
                    _ban_slashes[beg.neiEdge(j)] = 1;

            for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
                if (slashes[end.neiEdge(j)])
                    _ban_slashes[end.neiEdge(j)] = 1;
        }
    }
}

void SmilesSaver::_filterCisTransParity()
{
    BaseMolecule& mol = *_bmol;

    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        const Edge& edge = mol.getEdge(i);

        if (mol.cis_trans.getParity(i) != 0)
        {
            const Vertex& beg = mol.getVertex(edge.beg);
            const Vertex& end = mol.getVertex(edge.end);

            // Aromatic bonds can not be cis or trans. That is a limitation of the SMILES syntax.
            // Structures like N1\C=C/N\C=C/C=C/C=C\C=C\1, when aromatized,
            // become SMILES like c1cccc[nH]cc[nH]ccc1, and the cis-trans info is lost.

            // Rare structures like C1C=NC2=C1C=C(S2=CC3=CC=CN3)C(=O)[O-] (CID 17932462),
            // which have cis-trans double bond attached to an aromatic ring, lose the
            // cis-trans information too (although Marvin works that around saving to
            // N#Cc1cc2CC=Nc2\s1=C/c1ccc[nH]1).

            bool have_singlebond_beg = false, have_singlebond_end = false;
            //         bool have_allowed_singlebond_beg = false, have_allowed_singlebond_end = false;

            for (int j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
            {
                if (_ignored_vertices[beg.neiVertex(j)])
                    continue;
                int idx = beg.neiEdge(j);

                if (idx != i && mol.getBondOrder(idx) == BOND_SINGLE)
                    have_singlebond_beg = true;
            }

            for (int j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
            {
                if (_ignored_vertices[end.neiVertex(j)])
                    continue;
                int idx = end.neiEdge(j);

                if (idx != i && mol.getBondOrder(idx) == BOND_SINGLE)
                    have_singlebond_end = true;
            }

            if (!have_singlebond_beg || !have_singlebond_end)
                continue;

            if (mol.getBondTopology(i) == TOPOLOGY_RING)
            {
                // But cis-trans bonds can have only rings with size more then 7:
                // C1=C\C=C/C=C/C=C\1
                // C1=CC=CC=CC=C1
                // C1=C/C=C\C=C/C=C\1
                if (mol.edgeSmallestRingSize(i) <= 7)
                    continue;
            }

            // This bond will be saved as cis-trans
            _cis_trans_parity[i] = mol.cis_trans.getParity(i);
        }
    }
}

void SmilesSaver::_markCisTrans()
{
    BaseMolecule& mol = *_bmol;
    int i, j;

    _dbonds.clear_resize(mol.edgeEnd());

    for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        _dbonds[i].ctbond_beg = -1;
        _dbonds[i].ctbond_end = -1;
        _dbonds[i].saved = 0;
    }

    _filterCisTransParity();

    if (!mol.cis_trans.exists())
        return;

    _banSlashes();

    for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        const Edge& edge = mol.getEdge(i);

        if (_cis_trans_parity[i] != 0)
        {
            // This cis-trans bond has already been filtered in _filterCisTransParity
            // and we have to write cis-trans infromation for this bond
            const Vertex& beg = mol.getVertex(edge.beg);
            const Vertex& end = mol.getVertex(edge.end);

            bool have_allowed_singlebond_beg = false, have_allowed_singlebond_end = false;

            for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
            {
                if (_ignored_vertices[beg.neiVertex(j)])
                    continue;
                int idx = beg.neiEdge(j);

                if (idx != i && mol.getBondOrder(idx) == BOND_SINGLE)
                {
                    if (!_ban_slashes[idx])
                        have_allowed_singlebond_beg = true;
                }
            }

            for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
            {
                if (_ignored_vertices[end.neiVertex(j)])
                    continue;
                int idx = end.neiEdge(j);

                if (idx != i && mol.getBondOrder(idx) == BOND_SINGLE)
                {
                    if (!_ban_slashes[idx])
                        have_allowed_singlebond_end = true;
                }
            }

            if (!have_allowed_singlebond_beg || !have_allowed_singlebond_end)
            {
                _complicated_cistrans[i] = 1;
                _have_complicated_cistrans = true;
                continue;
            }

            for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
            {
                if (_ignored_vertices[beg.neiVertex(j)])
                    continue;
                int idx = beg.neiEdge(j);

                if (idx != i)
                {
                    if (mol.getEdge(idx).beg == edge.beg)
                        _dbonds[idx].ctbond_beg = i;
                    else
                        _dbonds[idx].ctbond_end = i;
                }
            }

            for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
            {
                if (_ignored_vertices[end.neiVertex(j)])
                    continue;
                int idx = end.neiEdge(j);

                if (idx != i)
                {
                    if (mol.getEdge(idx).beg == edge.end)
                        _dbonds[idx].ctbond_beg = i;
                    else
                        _dbonds[idx].ctbond_end = i;
                }
            }
        }
    }
}

bool SmilesSaver::_updateSideBonds(int bond_idx)
{
    BaseMolecule& mol = *_bmol;
    const Edge& edge = mol.getEdge(bond_idx);
    int subst[4];

    mol.getSubstituents_All(bond_idx, subst);
    int parity = _cis_trans_parity[bond_idx];

    int sidebonds[4] = {-1, -1, -1, -1};

    sidebonds[0] = mol.findEdgeIndex(subst[0], edge.beg);
    if (subst[1] != -1)
        sidebonds[1] = mol.findEdgeIndex(subst[1], edge.beg);

    sidebonds[2] = mol.findEdgeIndex(subst[2], edge.end);
    if (subst[3] != -1)
        sidebonds[3] = mol.findEdgeIndex(subst[3], edge.end);

    int n1 = 0, n2 = 0, n3 = 0, n4 = 0;

    if (_dbonds[sidebonds[0]].saved != 0)
    {
        if ((_dbonds[sidebonds[0]].saved == 1 && mol.getEdge(sidebonds[0]).beg == edge.beg) ||
            (_dbonds[sidebonds[0]].saved == 2 && mol.getEdge(sidebonds[0]).end == edge.beg))
            n1++;
        else
            n2++;
    }
    if (sidebonds[1] != -1 && _dbonds[sidebonds[1]].saved != 0)
    {
        if ((_dbonds[sidebonds[1]].saved == 2 && mol.getEdge(sidebonds[1]).beg == edge.beg) ||
            (_dbonds[sidebonds[1]].saved == 1 && mol.getEdge(sidebonds[1]).end == edge.beg))
            n1++;
        else
            n2++;
    }
    if (_dbonds[sidebonds[2]].saved != 0)
    {
        if ((_dbonds[sidebonds[2]].saved == 1 && mol.getEdge(sidebonds[2]).beg == edge.end) ||
            (_dbonds[sidebonds[2]].saved == 2 && mol.getEdge(sidebonds[2]).end == edge.end))
            n3++;
        else
            n4++;
    }
    if (sidebonds[3] != -1 && _dbonds[sidebonds[3]].saved != 0)
    {
        if ((_dbonds[sidebonds[3]].saved == 2 && mol.getEdge(sidebonds[3]).beg == edge.end) ||
            (_dbonds[sidebonds[3]].saved == 1 && mol.getEdge(sidebonds[3]).end == edge.end))
            n3++;
        else
            n4++;
    }

    if (parity == MoleculeCisTrans::CIS)
    {
        n1 += n3;
        n2 += n4;
    }
    else
    {
        n1 += n4;
        n2 += n3;
    }

    if (n1 > 0 && n2 > 0)
        throw Error("incompatible cis-trans configuration");

    if (n1 == 0 && n2 == 0)
        return false;

    if (n1 > 0)
    {
        _dbonds[sidebonds[0]].saved = (mol.getEdge(sidebonds[0]).beg == edge.beg) ? 1 : 2;

        if (sidebonds[1] != -1)
            _dbonds[sidebonds[1]].saved = (mol.getEdge(sidebonds[1]).beg == edge.beg) ? 2 : 1;

        _dbonds[sidebonds[2]].saved = ((mol.getEdge(sidebonds[2]).beg == edge.end) == (parity == MoleculeCisTrans::CIS)) ? 1 : 2;

        if (sidebonds[3] != -1)
            _dbonds[sidebonds[3]].saved = ((mol.getEdge(sidebonds[3]).beg == edge.end) == (parity == MoleculeCisTrans::CIS)) ? 2 : 1;
    }
    if (n2 > 0)
    {
        _dbonds[sidebonds[0]].saved = (mol.getEdge(sidebonds[0]).beg == edge.beg) ? 2 : 1;

        if (sidebonds[1] != -1)
            _dbonds[sidebonds[1]].saved = (mol.getEdge(sidebonds[1]).beg == edge.beg) ? 1 : 2;

        _dbonds[sidebonds[2]].saved = ((mol.getEdge(sidebonds[2]).beg == edge.end) == (parity == MoleculeCisTrans::CIS)) ? 2 : 1;

        if (sidebonds[3] != -1)
            _dbonds[sidebonds[3]].saved = ((mol.getEdge(sidebonds[3]).beg == edge.end) == (parity == MoleculeCisTrans::CIS)) ? 1 : 2;
    }

    return true;
}

int SmilesSaver::_calcBondDirection(int idx, int vprev)
{
    BaseMolecule& mol = *_bmol;
    int i, ntouched;
    int ctbond_beg = _dbonds[idx].ctbond_beg;
    int ctbond_end = _dbonds[idx].ctbond_end;

    if (ctbond_beg == -1 && ctbond_end == -1)
        return 0;

    if (mol.getBondOrder(idx) != BOND_SINGLE)
        throw Error("internal: directed bond order %d", mol.getBondOrder(idx));

    while (1)
    {
        ntouched = 0;
        for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
            if (_cis_trans_parity[i] != 0 && mol.getEdgeTopology(i) == TOPOLOGY_CHAIN)
            {
                if (_updateSideBonds(i))
                    ntouched++;
            }
        if (ntouched == _touched_cistransbonds)
            break;
        _touched_cistransbonds = ntouched;
    }

    if (_dbonds[idx].saved == 0)
    {
        if (vprev == mol.getEdge(idx).beg)
            _dbonds[idx].saved = 1;
        else
            _dbonds[idx].saved = 2;
    }

    return _dbonds[idx].saved;
}

void SmilesSaver::_startExtension()
{
    if (_comma)
        _output.writeChar(',');
    else
    {
        _output.writeString(" |");
        _comma = true;
    }
}

void SmilesSaver::_writeStereogroups()
{
    BaseMolecule& mol = *_bmol;
    MoleculeStereocenters& stereocenters = mol.stereocenters;
    int i, j;

    for (i = stereocenters.begin(); i != stereocenters.end(); i = stereocenters.next(i))
    {
        int atom, type, group;
        stereocenters.get(i, atom, type, group, 0);
        if (type != MoleculeStereocenters::ATOM_ABS)
            break;
    }

    if (i == stereocenters.end())
        return;

    int and_group_idx = 1;
    int or_group_idx = 1;

    QS_DEF(Array<int>, marked);

    marked.clear_resize(_written_atoms.size());
    marked.zerofill();

    for (i = 0; i < _written_atoms.size(); i++)
    {
        if (marked[i])
            continue;

        int type = stereocenters.getType(_written_atoms[i]);

        if (type == MoleculeStereocenters::ATOM_ANY)
        {
            _startExtension();
            _output.printf("w:%d", i);

            for (j = i + 1; j < _written_atoms.size(); j++)
                if (stereocenters.getType(_written_atoms[j]) == MoleculeStereocenters::ATOM_ANY)
                {
                    marked[j] = 1;
                    _output.printf(",%d", j);
                }
        }
        else
        {
            if (type == MoleculeStereocenters::ATOM_ABS)
            {
                _startExtension();
                _output.printf("a:%d", i);

                for (j = i + 1; j < _written_atoms.size(); j++)
                    if (stereocenters.getType(_written_atoms[j]) == MoleculeStereocenters::ATOM_ABS)
                    {
                        marked[j] = 1;
                        _output.printf(",%d", j);
                    }
            }
            else if (type == MoleculeStereocenters::ATOM_AND)
            {
                int group = stereocenters.getGroup(_written_atoms[i]);

                _startExtension();
                _output.printf("&%d:%d", and_group_idx++, i);
                for (j = i + 1; j < _written_atoms.size(); j++)
                    if (stereocenters.getType(_written_atoms[j]) == MoleculeStereocenters::ATOM_AND && stereocenters.getGroup(_written_atoms[j]) == group)
                    {
                        marked[j] = 1;
                        _output.printf(",%d", j);
                    }
            }
            else if (type == MoleculeStereocenters::ATOM_OR)
            {
                int group = stereocenters.getGroup(_written_atoms[i]);

                _startExtension();
                _output.printf("o%d:%d", or_group_idx++, i);
                for (j = i + 1; j < _written_atoms.size(); j++)
                    if (stereocenters.getType(_written_atoms[j]) == MoleculeStereocenters::ATOM_OR && stereocenters.getGroup(_written_atoms[j]) == group)
                    {
                        marked[j] = 1;
                        _output.printf(",%d", j);
                    }
            }
        }
    }

    if (!mol.isChiral())
        _output.printf(",r");
}

void SmilesSaver::_writeRadicals()
{
    BaseMolecule& mol = *_bmol;
    QS_DEF(Array<int>, marked);
    int i, j;

    marked.clear_resize(_written_atoms.size());
    marked.zerofill();

    for (i = 0; i < _written_atoms.size(); i++)
    {
        if (marked[i] || mol.isRSite(_written_atoms[i]) || mol.isPseudoAtom(_written_atoms[i]))
            continue;

        int radical = mol.getAtomRadical_NoThrow(_written_atoms[i], 0);

        if (radical <= 0)
            continue;

        _startExtension();

        if (radical == RADICAL_SINGLET)
            _output.writeString("^3:");
        else if (radical == RADICAL_DOUBLET)
            _output.writeString("^1:");
        else // RADICAL_TRIPLET
            _output.writeString("^4:");

        _output.printf("%d", i);

        for (j = i + 1; j < _written_atoms.size(); j++)
        {
            if (mol.isPseudoAtom(_written_atoms[j]) || mol.isRSite(_written_atoms[j]))
                continue;
            if (mol.getAtomRadical_NoThrow(_written_atoms[j], 0) == radical)
            {
                marked[j] = 1;
                _output.printf(",%d", j);
            }
        }
    }
}

void SmilesSaver::_writePseudoAtoms()
{
    BaseMolecule& mol = *_bmol;

    int i;

    if (_attachment_indices.size() == 0)
    {
        for (i = 0; i < _written_atoms.size(); i++)
        {
            if (mol.isAlias(_written_atoms[i]) || mol.isPseudoAtom(_written_atoms[i]) ||
                (mol.isRSite(_written_atoms[i]) && mol.getRSiteBits(_written_atoms[i]) != 0))
                break;
            if (_qmol != 0)
            {
                if (QueryMolecule::queryAtomIsSpecial(*_qmol, _written_atoms[i]))
                {
                    break;
                }
            }
        }

        if (i == _written_atoms.size())
            return;
    }

    _startExtension();

    _output.writeChar('$');

    for (i = 0; i < _written_atoms.size(); i++)
    {
        if (i > 0)
            _output.writeChar(';');

        if (mol.isPseudoAtom(_written_atoms[i]))
        {
            writePseudoAtom(mol.getPseudoAtom(_written_atoms[i]), _output);
        }
        else if (mol.isRSite(_written_atoms[i]) && mol.getRSiteBits(_written_atoms[i]) != 0)
        // ChemAxon's Extended SMILES notation for R-sites
        // and added support of multiple R-groups on one R-site
        {
            QS_DEF(Array<int>, allowed_rgroups);
            mol.getAllowedRGroups(_written_atoms[i], allowed_rgroups);
            for (int j = 0; j < allowed_rgroups.size(); j++)
            {
                _output.printf("_R%d", allowed_rgroups[j]);
                if (j < allowed_rgroups.size() - 1)
                    _output.printf(",");
            }
        }
        else if ((_qmol != 0) && (QueryMolecule::queryAtomIsSpecial(*_qmol, _written_atoms[i])))
        {
            writeSpecialAtom(_written_atoms[i], _output);
        }
        else if (mol.isAlias(_written_atoms[i]))
        {
            writePseudoAtom(mol.getAlias(_written_atoms[i]), _output);
        }
    }

    for (i = 0; i < _attachment_indices.size(); i++)
        // ChemAxon's Extended SMILES notation for attachment points
        _output.printf(";_AP%d", _attachment_indices[i]);

    _output.writeChar('$');
}

void SmilesSaver::writePseudoAtom(const char* label, Output& out)
{
    if (*label == 0)
        throw Error("empty pseudo-atom");

    do
    {
        if (*label == '\n' || *label == '\r' || *label == '\t')
            throw Error("character 0x%x is not allowed inside pseudo-atom", *label);
        if (*label == '$' || *label == ';')
            throw Error("'%c' not allowed inside pseudo-atom", *label);

        out.writeChar(*label);
    } while (*(++label) != 0);
    //   out.writeString("_p");
}

void SmilesSaver::writeSpecialAtom(int aid, Output& out)
{
    QS_DEF(Array<int>, list);
    int query_atom_type;

    query_atom_type = QueryMolecule::parseQueryAtom(*_qmol, aid, list);
    if (query_atom_type == QueryMolecule::QUERY_ATOM_Q)
        out.writeString("Q_e");
    else if (query_atom_type == QueryMolecule::QUERY_ATOM_X)
        out.writeString("X_p");
    else if (query_atom_type == QueryMolecule::QUERY_ATOM_M)
        out.writeString("M_p");
    else if (query_atom_type == QueryMolecule::QUERY_ATOM_AH)
        out.writeString("AH_p");
    else if (query_atom_type == QueryMolecule::QUERY_ATOM_QH)
        out.writeString("QH_p");
    else if (query_atom_type == QueryMolecule::QUERY_ATOM_XH)
        out.writeString("XH_p");
    else if (query_atom_type == QueryMolecule::QUERY_ATOM_MH)
        out.writeString("MH_p");
}

void SmilesSaver::_writeHighlighting()
{
    if (!_bmol->hasHighlighting())
        return;

    int i;

    bool ha = false;

    for (i = 0; i < _written_atoms.size(); i++)
    {
        if (_bmol->isAtomHighlighted(_written_atoms[i]))
        {
            if (ha)
                _output.writeChar(',');
            else
            {
                _startExtension();
                _output.writeString("ha:");
                ha = true;
            }

            _output.printf("%d", i);
        }
    }

    bool hb = false;

    for (i = 0; i < _written_bonds.size(); i++)
    {
        if (_bmol->isBondHighlighted(_written_bonds[i]))
        {
            if (hb)
                _output.writeChar(',');
            else
            {
                _startExtension();
                _output.writeString("hb:");
                hb = true;
            }

            _output.printf("%d", i);
        }
    }
}

void SmilesSaver::_writeUnsaturated()
{
    bool is_first = true;
    if (_qmol)
    {
        for (auto i : _bmol->vertices())
        {
            int unsat = 0;
            if (_qmol->getAtom(i).sureValue(QueryMolecule::ATOM_UNSATURATION, unsat))
            {
                if (is_first)
                {
                    _startExtension();
                    _output.writeString("u:");
                    is_first = false;
                }
                else
                    _output.writeString(",");
                _output.printf("%d", i);
            }
        }
    }
}

void SmilesSaver::_writeSubstitutionCounts()
{
    bool is_first = true;
    if (_qmol)
    {
        for (auto i : _bmol->vertices())
        {
            int subst = 0;
            if (MoleculeSavers::getSubstitutionCountFlagValue(*_qmol, i, subst))
            {
                if (is_first)
                {
                    _startExtension();
                    _output.writeString("s:");
                    is_first = false;
                }
                else
                    _output.writeString(",");
                switch (subst)
                {
                case -2:
                    _output.printf("%d:*", i);
                    break;
                case -1:
                    _output.printf("%d:0", i);
                    break;
                default:
                    _output.printf("%d:%d", i, subst);
                    break;
                }
            }
        }
    }
}

void SmilesSaver::_writeBondDirs(const std::string& tag, const std::vector<std::pair<int, int>>& bonds)
{
    bool is_first = true;
    for (const auto& kvp : bonds)
    {
        if (is_first)
        {
            _startExtension();
            _output.writeString(tag.c_str());
            is_first = false;
        }
        else
            _output.writeString(",");
        _output.printf("%d.%d", kvp.first, kvp.second);
    }
}

void SmilesSaver::_writeWedges()
{
    if (_bmol)
    {
        std::vector<std::pair<int, int>> down_dirs, up_dirs, wiggy_dirs;
        for (int i = 0; i < _written_bonds.size(); ++i)
        {
            auto bond_idx = _written_bonds[i];
            auto& e = _bmol->getEdge(bond_idx);
            auto bdir = _bmol->getBondDirection(bond_idx);
            if (bdir)
            {
                const auto& edge = _bmol->getEdge(bond_idx);
                auto wa_idx = _written_atoms.find(edge.beg);
                switch (bdir)
                {
                case BOND_UP:
                    up_dirs.emplace_back(wa_idx, i);
                    break;
                case BOND_DOWN:
                    down_dirs.emplace_back(wa_idx, i);
                    break;
                case BOND_EITHER:
                    wiggy_dirs.emplace_back(wa_idx, i);
                    break;
                }
            }
        }

        _writeBondDirs("wU:", up_dirs);
        _writeBondDirs("wD:", down_dirs);
        _writeBondDirs("w:", wiggy_dirs);

        if ((down_dirs.size() || up_dirs.size() || wiggy_dirs.size()) && BaseMolecule::hasCoord(*_mol))
        {
            _output.writeString(",(");
            for (int i = 0; i < _written_atoms.size(); ++i)
            {
                if (i)
                    _output.writeString(";");
                auto atom_idx = _written_atoms[i];
                const auto& pos = _mol->getAtomXyz(atom_idx);
                _output.printf("%.2f,%.2f,", pos.x, pos.y);
            }
            _output.writeString(")");
        }
    }
}

void SmilesSaver::_writeRingBonds()
{
    bool is_first = true;
    if (_qmol)
    {
        for (auto i : _qmol->vertices())
        {
            int rbc = 0;
            if (MoleculeSavers::getRingBondCountFlagValue(_qmol->asQueryMolecule(), i, rbc))
            {
                if (is_first)
                {
                    _startExtension();
                    _output.writeString("rb:");
                    is_first = false;
                }
                else
                    _output.writeString(",");
                if (rbc > 0)
                    _output.printf("%d:%d", i, rbc);
                else if (rbc == -2)
                    _output.printf("%d:*", i);
                else if (rbc == -1)
                    _output.printf("%d:0", i);
            }
        }
    }
}

void SmilesSaver::_writeSGroupAtoms(const SGroup& sgroup)
{
    for (int i = 0; i < sgroup.atoms.size(); ++i)
    {
        if (i)
            _output.printf(",");
        _output.printf("%d", sgroup.atoms[i]);
    }
}

void SmilesSaver::_writeSGroups()
{
    for (int i = _bmol->sgroups.begin(); i != _bmol->sgroups.end(); i = _bmol->sgroups.next(i))
    {
        SGroup& sg = _bmol->sgroups.getSGroup(i);
        if (!sg.atoms.size() || (sg.sgroup_type != SGroup::SG_TYPE_DAT && sg.sgroup_type != SGroup::SG_TYPE_GEN && sg.sgroup_type != SGroup::SG_TYPE_SRU))
            continue;
        _startExtension();
        _output.writeString(sg.sgroup_type == SGroup::SG_TYPE_DAT ? "SgD:" : "Sg:");
        switch (sg.sgroup_type)
        {
        case SGroup::SG_TYPE_DAT:
            _writeSGroupAtoms(sg);
            _output.writeChar(':');
            {
                DataSGroup& dsg = static_cast<DataSGroup&>(sg);
                if (dsg.name.size() > 0)
                    _output.writeString(dsg.name.ptr());
                _output.writeChar(':');
                if (dsg.data.size() > 0)
                    _output.writeString(dsg.data.ptr());
                _output.writeChar(':');
                if (dsg.queryoper.size() > 0)
                    _output.writeString(dsg.queryoper.ptr());
                _output.writeChar(':');
                if (dsg.description.size() > 0)
                    _output.writeString(dsg.description.ptr());
                _output.writeChar(':');
                _output.writeChar(dsg.tag);
                _output.writeChar(':');
                // No coords output for now
            }
            break;
        case SGroup::SG_TYPE_GEN:
            _output.writeString("gen:");
            _writeSGroupAtoms(sg);
            _output.writeString(":");
            break;
        case SGroup::SG_TYPE_SRU: {
            RepeatingUnit& ru = static_cast<RepeatingUnit&>(sg);
            _output.writeString("n:");
            _writeSGroupAtoms(sg);
            _output.printf(":%s:", ru.subscript.ptr() ? ru.subscript.ptr() : "");
            switch (ru.connectivity)
            {
            case SGroup::HEAD_TO_TAIL:
                _output.writeString("ht");
                break;
            case SGroup::HEAD_TO_HEAD:
                _output.writeString("hh");
                break;
            default:
                _output.writeString("eu");
                break;
            }
        }
        break;
        default:
            break;
        }
    }
}

void SmilesSaver::_writeRGroups()
{
    if (_bmol->rgroups.getRGroupCount() > 0)
    {
        MoleculeRGroups& rgroups = _bmol->rgroups;
        int n_rgroups = rgroups.getRGroupCount();
        bool first_rg = true;
        bool rlogic_found = false;

        for (int i = 1; i <= n_rgroups; i++)
        {
            RGroup& rgroup = rgroups.getRGroup(i);

            if (rgroup.fragments.size() == 0)
                continue;

            bool empty_fragments = true;
            PtrPool<BaseMolecule>& frags = rgroup.fragments;
            for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
            {
                BaseMolecule* fragment = frags[j];
                if (fragment->vertexCount() > 0)
                    empty_fragments = false;
            }
            if (empty_fragments)
                continue;

            if (first_rg)
            {
                _startExtension();
                _output.writeString("RG:");
                first_rg = false;
            }
            _output.printf("_R%d=", i);

            bool first_fr = true;

            for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
            {
                if (!first_fr)
                    _output.writeString(",");
                else
                    first_fr = false;

                BaseMolecule* fragment = frags[j];
                Array<char> out_buffer;
                ArrayOutput fr_out(out_buffer);

                if (ignore_hydrogens)
                {
                    CanonicalSmilesSaver fr_saver(fr_out);
                    if (_qmol != 0)
                        fr_saver.saveQueryMolecule(fragment->asQueryMolecule());
                    else
                        fr_saver.saveMolecule(fragment->asMolecule());
                }
                else
                {
                    SmilesSaver fr_saver(fr_out);
                    if (_qmol != 0)
                        fr_saver.saveQueryMolecule(fragment->asQueryMolecule());
                    else
                        fr_saver.saveMolecule(fragment->asMolecule());
                }

                _output.writeString("{");
                _output.writeArray(out_buffer);
                _output.writeString("}");
            }
            if (i < n_rgroups)
                _output.writeString(",");

            if ((rgroup.if_then > 0) || (rgroup.rest_h > 0) || (rgroup.occurrence.size() > 0))
                rlogic_found = true;
        }
        if (rlogic_found)
        {
            _output.writeString(",LOG={");

            for (int i = 1; i <= n_rgroups; i++)
            {
                if (i > 1)
                    _output.writeString(".");

                RGroup& rgroup = rgroups.getRGroup(i);
                _output.printf("_R%d:", i);

                if (rgroup.if_then > 0)
                    _output.printf("_R%d;", rgroup.if_then);
                else
                    _output.printf(";");

                if (rgroup.rest_h > 0)
                    _output.printf("H;");
                else
                    _output.printf(";");

                if (rgroup.occurrence.size() > 0)
                    _writeOccurrenceRanges(_output, rgroup.occurrence);
            }
            _output.writeString("}");
        }
    }
}

void SmilesSaver::_writeOccurrenceRanges(Output& out, const Array<int>& occurrences)
{
    for (int i = 0; i < occurrences.size(); i++)
    {
        int occurrence = occurrences[i];

        if ((occurrence & 0xFFFF) == 0xFFFF)
            out.printf(">%d", (occurrence >> 16) - 1);
        else if ((occurrence >> 16) == (occurrence & 0xFFFF))
            out.printf("%d", occurrence >> 16);
        else if ((occurrence >> 16) == 0)
            out.printf("<%d", (occurrence & 0xFFFF) + 1);
        else
            out.printf("%d-%d", occurrence >> 16, occurrence & 0xFFFF);

        if (i != occurrences.size() - 1)
            out.printf(",");
    }
}

int SmilesSaver::writtenComponents()
{
    return _written_components;
}

const Array<int>& SmilesSaver::writtenAtoms()
{
    return _written_atoms;
}

const Array<int>& SmilesSaver::writtenBonds()
{
    return _written_bonds;
}

SmilesSaver::_Atom::_Atom(Pool<List<int>::Elem>& neipool) : neighbors(neipool)
{
    aromatic = 0;
    lowercase = false;
    chirality = 0;
    branch_cnt = 0;
    paren_written = false;
    starts_polymer = false;
    ends_polymer = false;

    parent = -1;
}

SmilesSaver::_Atom::~_Atom()
{
}

void SmilesSaver::_checkSRU()
{
    _polymer_indices.clear_resize(_bmol->vertexEnd());
    _polymer_indices.fffill();

    if (chemaxon) // let's handle it in the extened block
        return;

    int i, j, k;

    // check overlapping (particularly nested) blocks
    for (i = _bmol->sgroups.begin(); i != _bmol->sgroups.end(); i = _bmol->sgroups.next(i))
    {
        auto& ru = (RepeatingUnit&)_bmol->sgroups.getSGroup(i);

        if (ru.sgroup_type == SGroup::SG_TYPE_SRU)
        {
            Array<int>& atoms = ru.atoms;

            for (j = 0; j < atoms.size(); j++)
            {
                if (_polymer_indices[atoms[j]] >= 0)
                    throw Error("overlapping (nested?) repeating units can not be saved");
                _polymer_indices[atoms[j]] = i;

                // check also disconnected blocks (possible to handle, but unsupported at the moment)
                if (_bmol->vertexComponent(atoms[j]) != _bmol->vertexComponent(atoms[0]))
                    throw Error("disconnected repeating units not supported");
            }
        }
    }

    // check that each block has exactly two outgoing bonds
    for (i = _bmol->sgroups.begin(); i != _bmol->sgroups.end(); i = _bmol->sgroups.next(i))
    {
        SGroup* sg = &_bmol->sgroups.getSGroup(i);
        if (sg->sgroup_type == SGroup::SG_TYPE_SRU)
        {
            Array<int>& atoms = sg->atoms;
            int cnt = 0;

            for (j = 0; j < atoms.size(); j++)
            {
                const Vertex& vertex = _bmol->getVertex(atoms[j]);
                for (k = vertex.neiBegin(); k != vertex.neiEnd(); k = vertex.neiNext(k))
                    if (_polymer_indices[vertex.neiVertex(k)] != i)
                        cnt++;
            }
            if (cnt != 2)
                throw Error("repeating units must have exactly two outgoing bonds, has %d", cnt);
        }
    }
}

int SmilesSaver::_countRBonds()
{
    int i, sum = 0;

    for (i = _bmol->vertexBegin(); i != _bmol->vertexEnd(); i = _bmol->vertexNext(i))
        if (_bmol->isRSite(i))
            sum += _bmol->getVertex(i).degree();
    return sum;
}

void SmilesSaver::_checkRGroupsAndAttachmentPoints()
{
    _attachment_indices.clear();
    _attachment_cycle_numbers.clear();

    _n_attachment_points = 0;

    for (int i = 1; i <= _bmol->attachmentPointCount(); i++)
        for (int idx = 0; _bmol->getAttachmentPoint(i, idx) != -1; idx++)
            _n_attachment_points++;

    if (_n_attachment_points && !write_extra_info)
        throw Error("can not write attachment points without permission to write "
                    "the Extended SMILES block (probably because you are saving reaction SMILES)");
}

void SmilesSaver::_writeRingCisTrans()
{
    if (!_have_complicated_cistrans)
        return;

    _startExtension();

    int i, j;

    QS_DEF(Array<int>, cis_bonds);
    QS_DEF(Array<int>, trans_bonds);

    cis_bonds.clear();
    trans_bonds.clear();

    for (i = 0; i < _written_bonds.size(); i++)
    {
        int bond_idx = _written_bonds[i];

        if (!_complicated_cistrans[bond_idx])
            continue;

        int nei_atom_beg = -1, nei_atom_end = -1;

        const Edge& edge = _bmol->getEdge(bond_idx);

        // TODO: a more effective looping is possible
        for (j = 0; j < _written_atoms.size(); j++)
        {
            if (nei_atom_beg == -1)
                if (_written_atoms[j] != edge.end && _bmol->findEdgeIndex(_written_atoms[j], edge.beg) >= 0)
                    nei_atom_beg = _written_atoms[j];
            if (nei_atom_end == -1)
                if (_written_atoms[j] != edge.beg && _bmol->findEdgeIndex(_written_atoms[j], edge.end) >= 0)
                    nei_atom_end = _written_atoms[j];
        }

        if (nei_atom_beg == -1 || nei_atom_end == -1)
            throw Error("_writeRingCisTrans(): internal");

        int parity = _cis_trans_parity[bond_idx];

        if (parity == 0 && _mol != 0 && _mol->getAtomRingBondsCount(edge.beg) == 2 && _mol->getAtomRingBondsCount(edge.end) == 2)
        {
            // That is some "simple" cis-trans bond, i.e. in a benzene ring.
            // It came unspecified, but we better write it explicitly, not to
            // produce different canonical SMILES for identical structures.
            // We write is as "cis" (if both substituents are in a ring),
            // or "trans" if one substituent is in a ring  and another is not.
            parity = MoleculeCisTrans::CIS;

            if (_bmol->getEdgeTopology(_bmol->findEdgeIndex(nei_atom_beg, edge.beg)) == TOPOLOGY_CHAIN)
                parity = 3 - parity;
            if (_bmol->getEdgeTopology(_bmol->findEdgeIndex(nei_atom_end, edge.end)) == TOPOLOGY_CHAIN)
                parity = 3 - parity;
        }
        else
        {
            const int* subst = _bmol->cis_trans.getSubstituents(bond_idx);

            if (nei_atom_beg == subst[0])
                ;
            else if (nei_atom_beg == subst[1])
                parity = 3 - parity;
            else
                throw Error("_writeRingCisTrans(): internal (substituent not found)");

            if (nei_atom_end == subst[2])
                ;
            else if (nei_atom_end == subst[3])
                parity = 3 - parity;
            else
                throw Error("_writeRingCisTrans(): internal (substituent not found)");
        }

        if (parity == MoleculeCisTrans::CIS)
            cis_bonds.push(i);
        else
            trans_bonds.push(i);
    }

    if (cis_bonds.size() != 0)
    {
        _output.printf("c:%d", cis_bonds[0]);
        for (i = 1; i < cis_bonds.size(); i++)
            _output.printf(",%d", cis_bonds[i]);
    }
    if (trans_bonds.size() != 0)
    {
        if (cis_bonds.size() != 0)
            _output.printf(",");

        _output.printf("t:%d", trans_bonds[0]);
        for (i = 1; i < trans_bonds.size(); i++)
            _output.printf(",%d", trans_bonds[i]);
    }
}

const Array<int>& SmilesSaver::getSavedCisTransParities()
{
    return _cis_trans_parity;
}

SmilesSaver::SMILES_MODE SmilesSaver::parseFormatMode(const std::string& format)
{
    if (format == "daylight")
        return SMILES_MODE::SMILES_DAYLIGHT;
    else if (format == "chemaxon")
        return SMILES_MODE::SMILES_CHEMAXON;
    else
        throw Error("unknown SMILES format: %s, supported values: chemaxon, daylight", format.c_str());
}

void SmilesSaver::saveFormatMode(SmilesSaver::SMILES_MODE mode, std::string& output)
{
    switch (mode)
    {
    case SMILES_MODE::SMILES_CHEMAXON:
        output = "chemaxon";
        break;
    case SMILES_MODE::SMILES_DAYLIGHT:
        output = "daylight";
        break;
    default:
        throw Error("unknown SMILES format mode: %d", mode);
    }
}
