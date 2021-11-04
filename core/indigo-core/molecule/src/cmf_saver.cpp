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

#include "molecule/cmf_saver.h"
#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "graph/dfs_walk.h"
#include "molecule/cmf_symbol_codes.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cis_trans.h"
#include "molecule/molecule_stereocenters.h"

using namespace indigo;

IMPL_ERROR(CmfSaver, "CMF saver");

CP_DEF(CmfSaver);

CmfSaver::CmfSaver(LzwDict& dict, Output& output) : CP_INIT, TL_CP_GET(_atom_sequence)
{
    _init();

    if (!dict.isInitialized())
        dict.init(CMF_ALPHABET_SIZE, CMF_BIT_CODE_SIZE);

    _encoder_obj.create(dict, output);
    _encoder_output_obj.create(_encoder_obj.ref());
    _output = _encoder_output_obj.get();
}

CmfSaver::CmfSaver(LzwEncoder& encoder) : CP_INIT, TL_CP_GET(_atom_sequence)
{
    _init();
    _ext_encoder = &encoder;
    _encoder_output_obj.create(encoder);
    _output = _encoder_output_obj.get();
}

CmfSaver::CmfSaver(Output& output) : CP_INIT, TL_CP_GET(_atom_sequence)
{
    _init();
    _output = &output;
}

void CmfSaver::_init()
{
    atom_flags = 0;
    bond_flags = 0;
    _mol = 0;
    _ext_encoder = 0;
    save_bond_dirs = false;
    save_highlighting = false;
    save_mapping = false;
}

void CmfSaver::saveMolecule(Molecule& mol)
{
    /* Walk molecule */
    DfsWalk walk(mol);
    QS_DEF(Array<int>, mapping);

    if (_ext_encoder != 0)
        _ext_encoder->start();

    walk.walk();

    /* Get walking sequence */
    const Array<DfsWalk::SeqElem>& v_seq = walk.getSequence();

    /* Calculate mapping to the encoded molecule */
    walk.calcMapping(mapping);

    QS_DEF(Array<int>, branch_counters);
    QS_DEF(Array<int>, cycle_numbers);

    branch_counters.clear_resize(mol.vertexEnd());
    branch_counters.zerofill();
    cycle_numbers.clear();

    _atom_sequence.clear();

    QS_DEF(Array<int>, bond_mapping);
    bond_mapping.clear_resize(mol.edgeEnd());
    bond_mapping.fffill();
    int bond_index = 0;

    /* Encode first atom */
    if (v_seq.size() > 0)
    {
        _encodeAtom(mol, v_seq[0].idx, mapping.ptr());
        _atom_sequence.push(v_seq[0].idx);

        int j, openings = walk.numOpenings(v_seq[0].idx);

        for (j = 0; j < openings; j++)
        {
            cycle_numbers.push(v_seq[0].idx);
            _encodeCycleNumer(j);
        }
    }

    /* Main cycle */
    int i, j, k;

    for (i = 1; i < v_seq.size(); i++)
    {
        int v_idx = v_seq[i].idx;
        int e_idx = v_seq[i].parent_edge;
        int v_prev_idx = v_seq[i].parent_vertex;
        bool write_atom = true;

        if (v_prev_idx >= 0)
        {
            if (walk.numBranches(v_prev_idx) > 1)
                if (branch_counters[v_prev_idx] > 0)
                    _encode(CMF_CLOSE_BRACKET);

            int branches = walk.numBranches(v_prev_idx);

            if (branches > 1)
                if (branch_counters[v_prev_idx] < branches - 1)
                    _encode(CMF_OPEN_BRACKET);

            branch_counters[v_prev_idx]++;

            if (branch_counters[v_prev_idx] > branches)
                throw Error("unexpected branch");

            _encodeBond(mol, e_idx, mapping.ptr());
            bond_mapping[e_idx] = bond_index++;

            if (save_bond_dirs)
            {
                int dir = mol.getBondDirection(e_idx);

                if (dir != 0)
                {
                    if (dir == BOND_UP)
                        dir = CMF_BOND_UP;
                    else if (dir == BOND_DOWN)
                        dir = CMF_BOND_DOWN;
                    else
                        dir = CMF_BOND_EITHER;

                    const Edge& edge = mol.getEdge(e_idx);

                    if (edge.beg == v_prev_idx && edge.end == v_idx)
                        ;
                    else if (edge.beg == v_idx && edge.end == v_prev_idx)
                        _encode(CMF_BOND_SWAP_ENDS);
                    else
                        throw Error("internal");

                    _encode(dir);
                }
            }

            if (walk.isClosure(e_idx))
            {
                for (j = 0; j < cycle_numbers.size(); j++)
                    if (cycle_numbers[j] == v_idx)
                        break;

                if (j == cycle_numbers.size())
                    throw Error("cycle number not found");

                _encodeCycleNumer(j);

                cycle_numbers[j] = -1;
                write_atom = false;
            }
        }
        else
            _encode(CMF_SEPARATOR);

        if (write_atom)
        {
            _encodeAtom(mol, v_idx, mapping.ptr());
            _atom_sequence.push(v_idx);

            int openings = walk.numOpenings(v_idx);

            for (j = 0; j < openings; j++)
            {
                for (k = 0; k < cycle_numbers.size(); k++)
                    if (cycle_numbers[k] == -1)
                        break;
                if (k == cycle_numbers.size())
                    cycle_numbers.push(v_idx);
                else
                    cycle_numbers[k] = v_idx;

                _encodeCycleNumer(k);
            }
        }
    }

    Mapping mapping_group;
    mapping_group.atom_mapping = &mapping;
    mapping_group.bond_mapping = &bond_mapping;
    _encodeExtSection(mol, mapping_group);

    _encode(CMF_TERMINATOR);

    // if have internal encoder, finish it
    if (_encoder_obj.get() != 0)
        _encoder_obj->finish();

    // for saveXyz()
    _mol = &mol;
}

void CmfSaver::_encodeUIntArray(const Array<int>& data, const Array<int>& mapping)
{
    _output->writePackedUInt(data.size());
    for (int i = 0; i < data.size(); i++)
    {
        int index = data[i];
        if (index < 0)
            throw Error("Internal error: index is invald: %d", index);
        int mapped = mapping[index];
        if (mapped < 0)
            throw Error("Internal error: mapping is invald");
        _output->writePackedUInt(mapped);
    }
}

void CmfSaver::_encodeUIntArray(const Array<int>& data)
{
    _output->writePackedUInt(data.size());
    for (int i = 0; i < data.size(); i++)
    {
        int index = data[i];
        if (index < 0)
            throw Error("Internal error: index is invald: %d", index);
        _output->writePackedUInt(index);
    }
}

void CmfSaver::_encodeUIntArraySkipNegative(const Array<int>& data)
{
    int len = 0;
    for (int i = 0; i < data.size(); i++)
        if (data[i] >= 0)
            len++;
    _output->writePackedUInt(len);
    for (int i = 0; i < data.size(); i++)
    {
        int index = data[i];
        if (index >= 0)
            _output->writePackedUInt(index);
    }
}

void CmfSaver::_encodeBaseSGroup(Molecule& mol, SGroup& sgroup, const Mapping& mapping)
{
    _encodeUIntArray(sgroup.atoms, *mapping.atom_mapping);
    _encodeUIntArray(sgroup.bonds, *mapping.bond_mapping);
}

void CmfSaver::_encodeExtSection(Molecule& mol, const Mapping& mapping)
{
    bool ext_printed = false;
    // Process all R-sites
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (mol.isRSite(i))
        {
            int count = 0;
            while (mol.getRSiteAttachmentPointByOrder(i, count) >= 0)
                count++;
            if (count == 0)
                continue;

            if (!ext_printed)
            {
                _encode(CMF_EXT);
                ext_printed = true;
            }

            _encode(CMF_RSITE_ATTACHMENTS);
            int idx = mapping.atom_mapping->at(i);
            if (idx < 0)
                throw Error("Internal error: idx < 0");
            _output->writePackedUInt(idx);
            _output->writePackedUInt(count);
            for (int j = 0; j < count; j++)
            {
                int att = mol.getRSiteAttachmentPointByOrder(i, j);
                int idx2 = mapping.atom_mapping->at(att);
                if (idx2 < 0)
                    throw Error("Internal error: idx2 < 0");
                _output->writePackedUInt(idx2);
            }
        }
    }

    bool need_print_ext = mol.sgroups.getSGroupCount() > 0;

    if (need_print_ext && !ext_printed)
    {
        _encode(CMF_EXT);
        ext_printed = true;
    }

    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sg = mol.sgroups.getSGroup(i);
        if (sg.sgroup_type == SGroup::SG_TYPE_GEN)
        {
            _encode(CMF_GENERICSGROUP);
            _encodeBaseSGroup(mol, sg, mapping);
        }
        else if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& sd = (DataSGroup&)sg;
            _encode(CMF_DATASGROUP);
            _encodeBaseSGroup(mol, sd, mapping);
            _encodeString(sd.description);
            _encodeString(sd.name);
            _encodeString(sd.type);
            _encodeString(sd.querycode);
            _encodeString(sd.queryoper);
            _encodeString(sd.data);
            // Pack detached, relative, display_units, and sd.dasp_pos into one byte
            if (sd.dasp_pos < 0 || sd.dasp_pos > 9)
                throw Error("DataSGroup dasp_pos field should be less than 10: %d", sd.dasp_pos);
            byte packed = (sd.dasp_pos & 0x0F) | (sd.detached << 4) | (sd.relative << 5) | (sd.display_units << 6);
            _output->writeByte(packed);
            _output->writePackedUInt(sd.num_chars);
            _output->writeChar(sd.tag);
        }
        else if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            Superatom& sa = (Superatom&)sg;
            _encode(CMF_SUPERATOM);
            _encodeBaseSGroup(mol, sa, mapping);
            _encodeString(sa.subscript);
            _encodeString(sa.sa_class);
            byte packed = (sa.contracted & 0x01) | (sa.bond_connections.size() << 1);
            _output->writeByte(packed);
            if (sa.bond_connections.size() > 0)
            {
                for (int j = 0; j < sa.bond_connections.size(); j++)
                {
                    _output->writePackedUInt(sa.bond_connections[j].bond_idx + 1);
                }
            }
        }
        else if (sg.sgroup_type == SGroup::SG_TYPE_SRU)
        {
            RepeatingUnit& su = (RepeatingUnit&)sg;
            _encode(CMF_REPEATINGUNIT);
            _encodeBaseSGroup(mol, su, mapping);
            _encodeString(su.subscript);
            _output->writePackedUInt(su.connectivity);
        }
        else if (sg.sgroup_type == SGroup::SG_TYPE_MUL)
        {
            MultipleGroup& sm = (MultipleGroup&)sg;
            _encode(CMF_MULTIPLEGROUP);
            _encodeBaseSGroup(mol, sm, mapping);
            _encodeUIntArray(sm.parent_atoms, *mapping.atom_mapping);
            if (sm.multiplier < 0)
                throw Error("internal error: SGroup multiplier is negative: %d", sm.multiplier);
            _output->writePackedUInt(sm.multiplier);
        }
    }

    // Encode mappings to restore
    if (save_mapping)
    {
        if (!ext_printed)
        {
            _encode(CMF_EXT);
            ext_printed = true;
        }
        _encode(CMF_MAPPING);

        _encodeUIntArraySkipNegative(*mapping.atom_mapping);
        _encodeUIntArraySkipNegative(*mapping.bond_mapping);
    }
}

void CmfSaver::_writeBaseSGroupXyz(Output& output, SGroup& sgroup, const VecRange& range)
{
    output.writePackedUInt(sgroup.brackets.size());
    for (int i = 0; i < sgroup.brackets.size(); i++)
    {
        _writeVec2f(output, sgroup.brackets[i][0], range);
        _writeVec2f(output, sgroup.brackets[i][1], range);
    }
}

void CmfSaver::_writeSGroupsXyz(Molecule& mol, Output& output, const VecRange& range)
{
    // XYZ data should be written in the same order as in _encodeSGroups
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sg = mol.sgroups.getSGroup(i);
        if ((sg.sgroup_type == SGroup::SG_TYPE_GEN) || (sg.sgroup_type == SGroup::SG_TYPE_SRU) || (sg.sgroup_type == SGroup::SG_TYPE_MUL))
        {
            _writeBaseSGroupXyz(output, sg, range);
        }
        else if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& sd = (DataSGroup&)sg;
            _writeBaseSGroupXyz(output, sd, range);
            _writeVec2f(output, sd.display_pos, range);
        }
        else if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            Superatom& sa = (Superatom&)sg;
            _writeBaseSGroupXyz(output, sa, range);
            if (sa.bond_connections.size() > 0)
            {
                for (int j = 0; j < sa.bond_connections.size(); j++)
                {
                    _writeDir2f(output, sa.bond_connections[j].bond_dir, range);
                }
            }
        }
    }
}

void CmfSaver::_encodeString(const Array<char>& str)
{
    unsigned int len = str.size();
    if (len > 0 && str[len - 1] == 0)
        len--;
    _output->writePackedUInt(len);
    _output->write(str.ptr(), len);
}

void CmfSaver::_encodeAtom(Molecule& mol, int idx, const int* mapping)
{
    int number = 0;

    if (mol.isPseudoAtom(idx))
    {
        const char* str = mol.getPseudoAtom(idx);
        size_t len = strlen(str);

        if (len < 1)
            throw Error("empty pseudo-atom");
        if (len > 255)
            throw Error("pseudo-atom labels %d characters long are not supported (255 is the limit)", len);

        _encode(CMF_PSEUDOATOM);
        _encode((byte)len);

        do
        {
            _encode(*str);
        } while (*(++str) != 0);
    }
    else if (mol.isRSite(idx))
    {
        int bits = mol.getRSiteBits(idx);
        if (bits > 255)
        {
            _encode(CMF_RSITE_EXT);
            _output->writePackedUInt((unsigned int)bits);
        }
        else
        {
            _encode(CMF_RSITE);
            _encode(bits);
        }
    }
    else
    {
        number = mol.getAtomNumber(idx);

        if (number <= 0 || number >= ELEM_MAX)
            throw Error("unexpected atom label");

        _encode(number);
    }

    int charge = mol.getAtomCharge(idx);

    if (charge != 0)
    {
        int charge2 = charge - CMF_MIN_CHARGE;

        if (charge2 < 0 || charge2 >= CMF_NUM_OF_CHARGES)
        {
            _encode(CMF_CHARGE_EXT);
            int charge3 = charge + 128;
            if (charge3 < 0 || charge >= 256)
                throw Error("unexpected atom charge: %d", charge);
            _encode(charge3);
        }
        else
            _encode(charge2 + CMF_CHARGES);
    }

    int isotope = mol.getAtomIsotope(idx);

    if (isotope > 0)
    {
        int deviation = isotope - Element::getDefaultIsotope(number);

        if (deviation == 0)
            _encode(CMF_ISOTOPE_ZERO);
        else if (deviation == 1)
            _encode(CMF_ISOTOPE_PLUS1);
        else if (deviation == 2)
            _encode(CMF_ISOTOPE_PLUS2);
        else if (deviation == -1)
            _encode(CMF_ISOTOPE_MINUS1);
        else if (deviation == -2)
            _encode(CMF_ISOTOPE_MINUS2);
        else
        {
            deviation += 100;
            if (deviation < 0 || deviation > 255)
            {
                throw Error("unexpected %s isotope: %d", Element::toString(number), isotope);
            }
            _encode(CMF_ISOTOPE_OTHER);
            _encode(deviation);
        }
    }

    int radical = 0;

    if (!mol.isPseudoAtom(idx) && !mol.isRSite(idx))
    {
        try
        {
            radical = mol.getAtomRadical(idx);
        }
        catch (Element::Error)
        {
        }
    }

    if (radical > 0)
    {
        if (radical == RADICAL_SINGLET)
            _encode(CMF_RADICAL_SINGLET);
        else if (radical == RADICAL_DOUBLET)
            _encode(CMF_RADICAL_DOUBLET);
        else if (radical == RADICAL_TRIPLET)
            _encode(CMF_RADICAL_TRIPLET);
        else
            throw Error("bad radical value: %d", radical);
    }

    MoleculeStereocenters& stereo = mol.stereocenters;

    int stereo_type = stereo.getType(idx);

    if (stereo_type == MoleculeStereocenters::ATOM_ANY)
        _encode(CMF_STEREO_ANY);
    else if (stereo_type != 0)
    {
        bool rigid;
        int code;
        const int* pyramid = stereo.getPyramid(idx);

        if (pyramid[3] == -1)
            rigid = MoleculeStereocenters::isPyramidMappingRigid(pyramid, 3, mapping);
        else
            rigid = MoleculeStereocenters::isPyramidMappingRigid(pyramid, 4, mapping);

        if (stereo_type == MoleculeStereocenters::ATOM_ABS)
            code = CMF_STEREO_ABS_0;
        else
        {
            int group = stereo.getGroup(idx);

            if (group < 1 || group > CMF_MAX_STEREOGROUPS)
                throw Error("stereogroup number %d out of range", group);

            if (stereo_type == MoleculeStereocenters::ATOM_AND)
                code = CMF_STEREO_AND_0 + group - 1;
            else // stereo_type == MoleculeStereocenters::ATOM_OR
                code = CMF_STEREO_OR_0 + group - 1;
        }

        if (!rigid)
            // CMF_STEREO_*_0 -> CMF_STEREO_*_1
            code += CMF_MAX_STEREOGROUPS * 2 + 1;

        _encode(code);
    }

    if (mol.allene_stereo.isCenter(idx))
    {
        int left, right, parity, subst[4];

        mol.allene_stereo.getByAtomIdx(idx, left, right, subst, parity);
        if (subst[1] != -1 && mapping[subst[1]] != -1 && mapping[subst[1]] < mapping[subst[0]])
            parity = 3 - parity;
        if (subst[3] != -1 && mapping[subst[3]] != -1 && mapping[subst[3]] < mapping[subst[2]])
            parity = 3 - parity;
        if (parity == 1)
            _encode(CMF_STEREO_ALLENE_0);
        else
            _encode(CMF_STEREO_ALLENE_1);
    }

    int impl_h = 0;

    if (!mol.isPseudoAtom(idx) && !mol.isRSite(idx) && Molecule::shouldWriteHCount(mol, idx))
    {
        try
        {
            impl_h = mol.getImplicitH(idx);

            if (impl_h < 0 || impl_h > CMF_MAX_IMPLICIT_H)
                throw Error("implicit hydrogen count %d out of range", impl_h);

            _encode(CMF_IMPLICIT_H + impl_h);
        }
        catch (Element::Error)
        {
        }
    }

    if (!mol.isRSite(idx) && !mol.isPseudoAtom(idx))
    {
        if (mol.isExplicitValenceSet(idx) || (mol.getAtomAromaticity(idx) == ATOM_AROMATIC && (charge != 0 || (number != ELEM_C && number != ELEM_O))))
        {
            try
            {
                int valence = mol.getAtomValence(idx);
                if (valence < 0 || valence > CMF_MAX_VALENCE)
                {
                    _encode(CMF_VALENCE_EXT);
                    _output->writePackedUInt(valence);
                }
                else
                    _encode(CMF_VALENCE + valence);
            }
            catch (Element::Error)
            {
            }
        }
    }

    int i;

    for (i = 1; i <= mol.attachmentPointCount(); i++)
    {
        int j, aidx;

        for (j = 0; (aidx = mol.getAttachmentPoint(i, j)) != -1; j++)
            if (aidx == idx)
            {
                _encode(CMF_ATTACHPT);
                _encode(i);
            }
    }

    if (atom_flags != 0)
    {
        int i, flags = atom_flags[idx];

        for (i = 0; i < CMF_NUM_OF_ATOM_FLAGS; i++)
            if (flags & (1 << i))
                _encode(CMF_ATOM_FLAGS + i);
    }

    if (save_highlighting)
        if (mol.isAtomHighlighted(idx))
            _encode(CMF_HIGHLIGHTED);
}

void CmfSaver::_encodeBond(Molecule& mol, int idx, const int* mapping)
{
    int order = mol.getBondOrder(idx);

    if (order == BOND_SINGLE)
    {
        if (mol.getBondTopology(idx) == TOPOLOGY_RING)
            _encode(CMF_BOND_SINGLE_RING);
        else
            _encode(CMF_BOND_SINGLE_CHAIN);
    }
    else if (order == BOND_DOUBLE)
    {
        int parity = mol.cis_trans.getParity(idx);

        if (parity != 0)
        {
            int mapped_parity = MoleculeCisTrans::applyMapping(parity, mol.cis_trans.getSubstituents(idx), mapping, true);

            if (mapped_parity == MoleculeCisTrans::CIS)
            {
                if (mol.getBondTopology(idx) == TOPOLOGY_RING)
                    _encode(CMF_BOND_DOUBLE_RING_CIS);
                else
                    _encode(CMF_BOND_DOUBLE_CHAIN_CIS);
            }
            else // mapped_parity == MoleculeCisTrans::TRANS
            {
                if (mol.getBondTopology(idx) == TOPOLOGY_RING)
                    _encode(CMF_BOND_DOUBLE_RING_TRANS);
                else
                    _encode(CMF_BOND_DOUBLE_CHAIN_TRANS);
            }
        }
        else if (mol.cis_trans.isIgnored(idx))
        {
            if (mol.getBondTopology(idx) == TOPOLOGY_RING)
                _encode(CMF_BOND_DOUBLE_IGNORED_CIS_TRANS_RING);
            else
                _encode(CMF_BOND_DOUBLE_IGNORED_CIS_TRANS_CHAIN);
        }
        else
        {
            if (mol.getBondTopology(idx) == TOPOLOGY_RING)
                _encode(CMF_BOND_DOUBLE_RING);
            else
                _encode(CMF_BOND_DOUBLE_CHAIN);
        }
    }
    else if (order == BOND_TRIPLE)
    {
        if (mol.getBondTopology(idx) == TOPOLOGY_RING)
            _encode(CMF_BOND_TRIPLE_RING);
        else
            _encode(CMF_BOND_TRIPLE_CHAIN);
    }
    else if (order == BOND_AROMATIC)
        _encode(CMF_BOND_AROMATIC);
    else
        throw Error("bad bond order: %d", order);

    if (bond_flags != 0)
    {
        int i, flags = bond_flags[idx];

        for (i = 0; i < CMF_NUM_OF_BOND_FLAGS; i++)
            if (flags & (1 << i))
                _encode(CMF_BOND_FLAGS + i);
    }

    if (save_highlighting)
        if (mol.isBondHighlighted(idx))
            _encode(CMF_HIGHLIGHTED);
}

void CmfSaver::_encodeCycleNumer(int n)
{
    while (n >= CMF_NUM_OF_CYCLES)
    {
        _encode(CMF_CYCLES_PLUS);
        n -= CMF_NUM_OF_CYCLES;
    }
    _encode(CMF_CYCLES + n);
}

void CmfSaver::_encode(byte symbol)
{
    _output->writeByte(symbol);
}

void CmfSaver::_writeFloatInRange(Output& output, float v, float min, float range)
{
    if (range > EPSILON)
    {
        float v2 = (((v - min) / range) * 65535 + 0.5f);
        if (v2 < 0 || v2 > 65536)
            throw Error("Internal error: Value %f is outsize of [%f, %f]", v, min, min + range);
        output.writeBinaryWord((word)v2);
    }
    else
        output.writeBinaryWord(0);
}

void CmfSaver::_writeVec3f(Output& output, const Vec3f& pos, const VecRange& range)
{
    _writeFloatInRange(output, pos.x, range.xyz_min.x, range.xyz_range.x);
    _writeFloatInRange(output, pos.y, range.xyz_min.y, range.xyz_range.y);
    if (range.have_z)
        _writeFloatInRange(output, pos.z, range.xyz_min.z, range.xyz_range.z);
}

void CmfSaver::_writeVec2f(Output& output, const Vec2f& pos, const VecRange& range)
{
    _writeFloatInRange(output, pos.x, range.xyz_min.x, range.xyz_range.x);
    _writeFloatInRange(output, pos.y, range.xyz_min.y, range.xyz_range.y);
}

void CmfSaver::_writeDir2f(Output& output, const Vec2f& dir, const VecRange& range)
{
    _writeFloatInRange(output, dir.x, -range.xyz_range.x, 2 * range.xyz_range.x);
    _writeFloatInRange(output, dir.y, -range.xyz_range.y, 2 * range.xyz_range.y);
}

void CmfSaver::_updateSGroupsXyzMinMax(Molecule& mol, Vec3f& min, Vec3f& max)
{
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sg = mol.sgroups.getSGroup(i);
        if ((sg.sgroup_type == SGroup::SG_TYPE_SUP) || (sg.sgroup_type == SGroup::SG_TYPE_SRU) || (sg.sgroup_type == SGroup::SG_TYPE_MUL) ||
            (sg.sgroup_type == SGroup::SG_TYPE_GEN))
        {
            _updateBaseSGroupXyzMinMax(sg, min, max);
        }
        else if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& s = (DataSGroup&)sg;
            _updateBaseSGroupXyzMinMax(s, min, max);

            Vec3f display_pos(s.display_pos.x, s.display_pos.y, 0);

            min.min(display_pos);
            max.max(display_pos);
        }
    }
}

void CmfSaver::_updateBaseSGroupXyzMinMax(SGroup& sgroup, Vec3f& min, Vec3f& max)
{
    for (int i = 0; i < sgroup.brackets.size(); i++)
    {
        Vec2f v1 = sgroup.brackets[i][0];
        Vec2f v2 = sgroup.brackets[i][1];

        Vec3f v31(v1.x, v1.y, 0);
        Vec3f v32(v2.x, v2.y, 0);

        min.min(v31);
        max.max(v31);

        min.min(v32);
        max.max(v32);
    }
}

void CmfSaver::saveXyz(Output& output)
{
    int i;

    if (_mol == 0)
        throw Error("saveMolecule() must be called prior to saveXyz()");

    if (!_mol->have_xyz)
        throw Error("saveXyz(): molecule has no XYZ");

    Vec3f xyz_min(10000, 10000, 10000);
    Vec3f xyz_max(-10000, -10000, -10000);

    VecRange range;

    for (i = 0; i < _atom_sequence.size(); i++)
    {
        const Vec3f& pos = _mol->getAtomXyz(_atom_sequence[i]);

        xyz_min.min(pos);
        xyz_max.max(pos);
    }
    _updateSGroupsXyzMinMax(*_mol, xyz_min, xyz_max);

    range.xyz_min = xyz_min;
    range.xyz_range.diff(xyz_max, xyz_min);

    output.writeBinaryFloat(range.xyz_min.x);
    output.writeBinaryFloat(range.xyz_min.y);
    output.writeBinaryFloat(range.xyz_min.z);
    output.writeBinaryFloat(range.xyz_range.x);
    output.writeBinaryFloat(range.xyz_range.y);
    output.writeBinaryFloat(range.xyz_range.z);

    if (range.xyz_range.z < EPSILON)
    {
        range.have_z = false;
        output.writeByte(0);
    }
    else
    {
        range.have_z = true;
        output.writeByte(1);
    }

    for (i = 0; i < _atom_sequence.size(); i++)
    {
        const Vec3f& pos = _mol->getAtomXyz(_atom_sequence[i]);
        _writeVec3f(output, pos, range);
    }
    _writeSGroupsXyz(*_mol, output, range);
}

const Array<int>& CmfSaver::getAtomSequence()
{
    return _atom_sequence;
}
