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

#include <memory>

#include "../layout/molecule_layout.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "layout/sequence_layout.h"
#include "molecule/elements.h"
#include "molecule/inchi_wrapper.h"
#include "molecule/molecule.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "molecule/monomer_commons.h"
#include "molecule/parse_utils.h"
#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"

#define STRCMP(a, b) strncmp((a), (b), strlen(b))

using namespace indigo;

void MolfileLoader::_postLoad()
{
    for (int i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
    {
        // Update attachment orders for rgroup bonds if they were not specified explicitly
        if (!_bmol->isRSite(i))
            continue;

        const Vertex& vertex = _bmol->getVertex(i);

        if (vertex.degree() == 1 && _bmol->getRSiteAttachmentPointByOrder(i, 0) == -1)
            _bmol->setRSiteAttachmentOrder(i, vertex.neiVertex(vertex.neiBegin()), 0);
        else if (vertex.degree() == 2 && (_bmol->getRSiteAttachmentPointByOrder(i, 0) == -1 || _bmol->getRSiteAttachmentPointByOrder(i, 1) == -1))
        {
            int nei_idx_1 = vertex.neiVertex(vertex.neiBegin());
            int nei_idx_2 = vertex.neiVertex(vertex.neiNext(vertex.neiBegin()));

            _bmol->setRSiteAttachmentOrder(i, std::min(nei_idx_1, nei_idx_2), 0);
            _bmol->setRSiteAttachmentOrder(i, std::max(nei_idx_1, nei_idx_2), 1);
        }
        else if (vertex.degree() > 2)
        {
            int j;

            for (j = 0; j < vertex.degree(); j++)
                if (_bmol->getRSiteAttachmentPointByOrder(i, j) == -1)
                {
                    QS_DEF(Array<int>, nei_indices);
                    nei_indices.clear();

                    for (int nei_idx = vertex.neiBegin(); nei_idx < vertex.neiEnd(); nei_idx = vertex.neiNext(nei_idx))
                        nei_indices.push(vertex.neiVertex(nei_idx));

                    nei_indices.qsort(_asc_cmp_cb, 0);

                    for (int order = 0; order < vertex.degree(); order++)
                        _bmol->setRSiteAttachmentOrder(i, nei_indices[order], order);
                    break;
                }
        }
    }

    if (_mol != 0)
    {
        for (int k = 0; k < _atoms_num; k++)
            if (_hcount[k] > 0)
                _mol->setImplicitH(k, _hcount[k] - 1);
    }

    for (int i = _bmol->sgroups.begin(); i != _bmol->sgroups.end(); i = _bmol->sgroups.next(i))
    {
        SGroup& sgroup = _bmol->sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& dsg = static_cast<DataSGroup&>(sgroup);
            if (dsg.parent_idx > -1 && std::string(dsg.name.ptr()) == "SMMX:class")
            {
                SGroup& parent_sgroup = _bmol->sgroups.getSGroup(dsg.parent_idx);
                if (parent_sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
                {
                    auto& sa = static_cast<Superatom&>(parent_sgroup);
                    if (sa.sa_natreplace.size() == 0)
                        sa.sa_natreplace.copy(dsg.sa_natreplace);
                    if (sa.sa_class.size() == 0)
                        sa.sa_class.copy(dsg.data);
                }
            }

            if (dsg.isMrv_implicit())
            {
                BufferScanner scanner(dsg.data);
                scanner.skip(DataSGroup::impl_prefix_len); // IMPL_H
                int hcount = scanner.readInt1();
                int k = dsg.atoms[0];

                if ((_mol != 0) && (_hcount[k] == 0))
                    _mol->setImplicitH(k, hcount);
                else if (_hcount[k] == 0)
                {
                    _hcount[k] = hcount + 1;
                }
                _bmol->sgroups.remove(i);
            }
        }
    }

    if (_qmol != 0)
    {
        for (int i = _qmol->edgeBegin(); i < _qmol->edgeEnd(); i = _qmol->edgeNext(i))
        {
            if (_ignore_cistrans[i])
                continue;

            const Edge& edge = _qmol->getEdge(i);

            if ((_stereo_care_atoms[edge.beg] && _stereo_care_atoms[edge.end]) || _stereo_care_bonds[i])
            {
                // in fragments such as C-C=C-C=C-C, the middle single bond
                // has both ends 'stereo care', but should not be considered
                // as 'stereo care' itself
                if (MoleculeCisTrans::isGeomStereoBond(*_bmol, i, 0, true))
                    _qmol->setBondStereoCare(i, true);
            }
        }

        for (int k = 0; k < _atoms_num; k++)
        {
            int expl_h = 0;

            if (_hcount[k] >= 0)
            {
                // count explicit hydrogens
                const Vertex& vertex = _bmol->getVertex(k);
                for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                {
                    if (_bmol->getAtomNumber(vertex.neiVertex(j)) == ELEM_H)
                        expl_h++;
                }
            }

            if (_hcount[k] == 1)
            {
                // no hydrogens unless explicitly drawn
                _qmol->resetAtom(k, QueryMolecule::Atom::und(_qmol->releaseAtom(k), new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_H, expl_h)));
            }
            else if (_hcount[k] > 1)
            {
                // (_hcount[k] - 1) or more atoms in addition to explicitly drawn
                // no hydrogens unless explicitly drawn
                _qmol->resetAtom(
                    k, QueryMolecule::Atom::und(_qmol->releaseAtom(k), new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_H, expl_h + _hcount[k] - 1, 100)));
            }
        }
    }

    // Some "either" bonds may mean not "either stereocenter", but
    // "either cis-trans", or "connected to either cis-trans".

    for (int i = 0; i < _bonds_num; i++)
        if (_bmol->getBondDirection(i) == BOND_EITHER)
        {
            if (MoleculeCisTrans::isGeomStereoBond(*_bmol, i, 0, true))
            {
                _ignore_cistrans[i] = 1;
                _sensible_bond_directions[i] = 1;
            }
            else
            {
                int k;
                const Vertex& v = _bmol->getVertex(_bmol->getEdge(i).beg);

                for (k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                {
                    if (MoleculeCisTrans::isGeomStereoBond(*_bmol, v.neiEdge(k), 0, true))
                    {
                        _ignore_cistrans[v.neiEdge(k)] = 1;
                        _sensible_bond_directions[i] = 1;
                        break;
                    }
                }
            }
        }

    _bmol->buildFromBondsStereocenters(stereochemistry_options, _sensible_bond_directions.ptr());
    _bmol->buildFromBondsAlleneStereo(stereochemistry_options.ignore_errors, _sensible_bond_directions.ptr());

    if (!_chiral && treat_stereo_as == 0)
        for (int i = 0; i < _atoms_num; i++)
        {
            int type = _bmol->stereocenters.getType(i);

            if (type == MoleculeStereocenters::ATOM_ABS)
                _bmol->stereocenters.setType(i, MoleculeStereocenters::ATOM_AND, 1);
        }

    if (treat_stereo_as != 0)
        for (int i = 0; i < _atoms_num; i++)
        {
            int type = _bmol->stereocenters.getType(i);

            if (type == MoleculeStereocenters::ATOM_ABS)
                _bmol->stereocenters.setType(i, treat_stereo_as, 1);
        }

    for (int i = 0; i < _atoms_num; i++)
        if (_stereocenter_types[i] > 0)
        {
            if (_bmol->stereocenters.getType(i) == 0)
            {
                if (stereochemistry_options.ignore_errors)
                    _bmol->addStereocentersIgnoreBad(i, _stereocenter_types[i], _stereocenter_groups[i], false); // add non-valid stereocenters
                else if (_qmol == nullptr)
                    throw Error("stereo type specified for atom #%d, but the bond "
                                "directions does not say that it is a stereocenter",
                                i);
            }
            else
                _bmol->stereocenters.setType(i, _stereocenter_types[i], _stereocenter_groups[i]);
        }

    _bmol->buildCisTrans(_ignore_cistrans.ptr());

    for (int i = 0; i < _bonds_num; i++)
    {
        if (_bmol->getBondDirection(i) && !_sensible_bond_directions[i])
        {
            if (!stereochemistry_options.ignore_errors && !_qmol) // Don't check for query molecule
                throw Error("direction of bond #%d makes no sense", i);
        }
    }

    // Remove adding default R-group logic behavior
    /*
       int n_rgroups = _bmol->rgroups.getRGroupCount();
       for (i = 1; i <= n_rgroups; i++)
          if (_bmol->rgroups.getRGroup(i).occurrence.size() == 0 &&
              _bmol->rgroups.getRGroup(i).fragments.size() > 0)
             _bmol->rgroups.getRGroup(i).occurrence.push((1 << 16) | 0xFFFF);
    */
    _bmol->have_xyz = true;
    MoleculeCIPCalculator cip;
    cip.convertSGroupsToCIP(*_bmol);

    // collect unique nucleotide templates

    std::unordered_map<std::string, int> nucleo_templates;
    for (int tg_idx = _bmol->tgroups.begin(); tg_idx != _bmol->tgroups.end(); tg_idx = _bmol->tgroups.next(tg_idx))
    {
        auto& tg = _bmol->tgroups.getTGroup(tg_idx);
        if (isNucleotideClass(tg.tgroup_class.ptr()))
            nucleo_templates.emplace(tg.tgroup_name.ptr(), tg_idx);
    }

    if (!_disable_sgroups_conversion && _bmol->sgroups.getSGroupCount() && (_bmol->tgroups.getTGroupCount() || _monomer_library))
        _bmol->transformSuperatomsToTemplates(_max_template_id, _monomer_library);

    std::set<int> templates_to_remove;
    std::unordered_map<MonomerKey, int, pair_hash> new_templates;
    if (!_bmol->isQueryMolecule())
    {
        for (int atom_idx = _bmol->vertexBegin(); atom_idx != _bmol->vertexEnd(); atom_idx = _bmol->vertexNext(atom_idx))
        {
            if (_bmol->isTemplateAtom(atom_idx) && isNucleotideClass(_bmol->getTemplateAtomClass(atom_idx)))
            {
                int tg_idx = _bmol->getTemplateAtomTemplateIndex(atom_idx);
                if (tg_idx == -1)
                {
                    auto atom_name = _bmol->getTemplateAtom(atom_idx);
                    auto nt_it = nucleo_templates.find(atom_name);
                    if (nt_it != nucleo_templates.end())
                    {
                        if (_expandNucleotide(atom_idx, nt_it->second, new_templates))
                            templates_to_remove.insert(nt_it->second);
                    }
                    else
                    {
                        // TODO: handle missing template case
                    }
                }
                else // tg_idx != -1 means the template is converted from S-Group
                {
                    // TODO: handle modified monomer. Currently it leaves as is.
                }
            }
        }
    }

    for (auto idx : templates_to_remove)
        _bmol->tgroups.remove(idx);

    // fix layout
    // if (templates_to_remove.size() && _bmol->countComponents())
    //{
    //    SequenceLayout sl(*_bmol);
    //    sl.make();
    //}
}

int MolfileLoader::_insertTemplate(MonomersLib::value_type& nuc, std::unordered_map<MonomerKey, int, pair_hash>& new_templates)
{
    auto it = new_templates.find(nuc.first);
    if (it != new_templates.end())
        return it->second;
    int tg_idx = _bmol->tgroups.addTGroup();
    auto& tg = _bmol->tgroups.getTGroup(tg_idx);
    // handle tgroup
    tg.copy(nuc.second);
    tg.tgroup_id = tg_idx + 1; // tgroup_id is 1-based
    new_templates.emplace(nuc.first, tg_idx);
    return tg_idx;
}

bool MolfileLoader::_expandNucleotide(int nuc_atom_idx, int tg_idx, std::unordered_map<MonomerKey, int, pair_hash>& new_templates)
{
    // get tgroup associated with atom_idx
    auto& tg = _bmol->tgroups.getTGroup(tg_idx);
    GranularNucleotide nuc;
    if (MonomerTemplates::splitNucleotide(tg.tgroup_class.ptr(), tg.tgroup_name.ptr(), nuc))
    {
        auto& ph = nuc.at(MonomerClass::Phosphate).get();
        auto& sugar = nuc.at(MonomerClass::Sugar).get();
        auto& base = nuc.at(MonomerClass::Base).get();
        int seq_id = _bmol->getTemplateAtomSeqid(nuc_atom_idx);
        // collect attachment points. only left attachment remains untouched.
        std::unordered_map<std::string, int> atp_map;
        for (int j = _bmol->template_attachment_points.begin(); j != _bmol->template_attachment_points.end(); j = _bmol->template_attachment_points.next(j))
        {
            auto& ap = _bmol->template_attachment_points.at(j);
            if (ap.ap_occur_idx == nuc_atom_idx && std::string(ap.ap_id.ptr()) > kLeftAttachmentPoint)
            {
                atp_map.emplace(ap.ap_id.ptr(), ap.ap_aidx);
                _bmol->template_attachment_points.remove(j);
                auto att_idxs = _bmol->getTemplateAtomAttachmentPointIdxs(nuc_atom_idx, j);
                if (att_idxs.has_value())
                    att_idxs.value().second.get().remove(att_idxs.value().first);
            }
        }

        // patch existing nucleotide atom with phosphate
        _bmol->renameTemplateAtom(nuc_atom_idx, ph.first.second.c_str());
        _bmol->setTemplateAtomClass(nuc_atom_idx, MonomerTemplates::classToStr(ph.first.first).c_str());

        // add sugar
        int sugar_idx = _bmol->addTemplateAtom(sugar.first.second.c_str());
        _bmol->setTemplateAtomClass(sugar_idx, MonomerTemplates::classToStr(sugar.first.first).c_str());

        // add base
        int base_idx = _bmol->addTemplateAtom(base.first.second.c_str());
        _bmol->setTemplateAtomClass(base_idx, MonomerTemplates::classToStr(base.first.first).c_str());

        // modify connections
        auto right_it = atp_map.find(std::string(kRightAttachmentPoint));
        int right_idx = -1;
        if (right_it != atp_map.end())
        {
            // nucleotide had Br attachment point. Now it should be moved to sugar.
            //   disconnect right nucleotide
            right_idx = right_it->second;
            _bmol->removeEdge(_bmol->findEdgeIndex(nuc_atom_idx, right_idx));
            // connect right nucleotide to the sugar
            _bmol->addBond_Silent(sugar_idx, right_it->second, BOND_SINGLE);
            // [sugar <- (Al) right nucleotide]
            _bmol->updateTemplateAtomAttachmentDestination(right_idx, nuc_atom_idx, sugar_idx);
            // [sugar (Br) -> right nucleotide]
            _bmol->setTemplateAtomAttachmentOrder(sugar_idx, right_idx, kRightAttachmentPoint);
            atp_map.erase(right_it);
        }

        for (auto& atp : atp_map)
        {
            _bmol->removeEdge(_bmol->findEdgeIndex(nuc_atom_idx, atp.second));
            // connect branch to base. which is incorrect!!! TODO: use substructure matcher to determine right monomer!!!
            _bmol->addBond_Silent(base_idx, atp.second, BOND_SINGLE);
            // [sugar <- (Al) right nucleotide]
            _bmol->updateTemplateAtomAttachmentDestination(atp.second, nuc_atom_idx, base_idx);
            // [sugar (Br) -> right nucleotide]
            _bmol->setTemplateAtomAttachmentOrder(base_idx, right_it->second, atp.first.c_str());
        }

        // connect phosphate to the sugar
        _bmol->addBond_Silent(nuc_atom_idx, sugar_idx, BOND_SINGLE);
        // [phosphate (Br) -> sugar]
        _bmol->setTemplateAtomAttachmentOrder(nuc_atom_idx, sugar_idx, kRightAttachmentPoint);
        // [phosphate <- (Al) sugar]
        _bmol->setTemplateAtomAttachmentOrder(sugar_idx, nuc_atom_idx, kLeftAttachmentPoint);
        // connect base to sugar
        _bmol->addBond_Silent(sugar_idx, base_idx, BOND_SINGLE);
        // [sugar (Cx) -> base]
        _bmol->setTemplateAtomAttachmentOrder(sugar_idx, base_idx, kBranchAttachmentPoint);
        // [sugar <- (Al) base]
        _bmol->setTemplateAtomAttachmentOrder(base_idx, sugar_idx, kLeftAttachmentPoint);
        // fix coordinates
        Vec3f sugar_pos, base_pos;
        if (!_bmol->getMiddlePoint(nuc_atom_idx, right_idx, sugar_pos))
        {
            sugar_pos.copy(_bmol->getAtomXyz(nuc_atom_idx));
            sugar_pos.x += LayoutOptions::DEFAULT_BOND_LENGTH;
        }
        base_pos.copy(sugar_pos);
        base_pos.y -= LayoutOptions::DEFAULT_BOND_LENGTH;
        _bmol->setAtomXyz(sugar_idx, sugar_pos);
        _bmol->setAtomXyz(base_idx, base_pos);
        // set seqid
        _bmol->setTemplateAtomSeqid(nuc_atom_idx, seq_id++); // increment seq_id after phosphate
        _bmol->setTemplateAtomSeqid(sugar_idx, seq_id);
        _bmol->setTemplateAtomSeqid(base_idx, seq_id);
        // handle templates
        _bmol->setTemplateAtomTemplateIndex(nuc_atom_idx, _insertTemplate(ph, new_templates));
        _bmol->setTemplateAtomTemplateIndex(sugar_idx, _insertTemplate(sugar, new_templates));
        _bmol->setTemplateAtomTemplateIndex(base_idx, _insertTemplate(base, new_templates));
        return true;
    }
    return false;
}

void MolfileLoader::_readRGroups2000()
{
    MoleculeRGroups* rgroups = &_bmol->rgroups;

    // read groups
    while (!_scanner.isEOF())
    {
        char chars[5];
        chars[4] = 0;
        _scanner.readCharsFix(4, chars);

        if (strncmp(chars, "$RGP", 4) == 0)
        {
            _scanner.skipLine();
            _scanner.skipSpace();

            int rgroup_idx = _scanner.readInt();
            RGroup& rgroup = rgroups->getRGroup(rgroup_idx);

            _scanner.skipLine();
            while (!_scanner.isEOF())
            {
                char rgp_chars[6];
                rgp_chars[5] = 0;
                _scanner.readCharsFix(5, rgp_chars);

                if (strncmp(rgp_chars, "$CTAB", 5) == 0)
                {
                    _scanner.skipLine();
                    std::unique_ptr<BaseMolecule> fragment(_bmol->neu());

                    MolfileLoader loader(_scanner);

                    loader._bmol = fragment.get();
                    if (_bmol->isQueryMolecule())
                    {
                        loader._qmol = &loader._bmol->asQueryMolecule();
                        loader._mol = 0;
                    }
                    else
                    {
                        loader._mol = &loader._bmol->asMolecule();
                        loader._qmol = 0;
                    }
                    loader._readCtabHeader();
                    loader._readCtab2000();
                    if (loader._rgfile)
                        loader._readRGroups2000();
                    loader._postLoad();

                    rgroup.fragments.add(fragment.release());
                }
                else if (strncmp(rgp_chars, "$END ", 5) == 0)
                {
                    rgp_chars[3] = 0;
                    _scanner.readCharsFix(3, rgp_chars);

                    _scanner.skipLine();
                    if (strncmp(rgp_chars, "RGP", 3) == 0)
                        break;
                }
                else
                    _scanner.skipLine();
            }
        }
        else if (strncmp(chars, "$END", 4) == 0)
        {
            chars[4] = 0;
            _scanner.readCharsFix(4, chars);
            _scanner.skipLine();
            if (strncmp(chars, " MOL", 4) == 0)
                break;
        }
        else
            _scanner.skipLine();
    }
}

