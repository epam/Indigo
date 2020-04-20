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

#ifndef __cmf_loader_h__
#define __cmf_loader_h__

#include "base_cpp/bitinworker.h"
#include "base_cpp/obj.h"
#include "lzw/lzw_decoder.h"
#include "lzw/lzw_dictionary.h"
#include "molecule/cmf_saver.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Molecule;
    class Scanner;
    struct Vec3f;

    class DLLEXPORT CmfLoader
    {
    public:
        // external dictionary, internal decoder
        explicit CmfLoader(LzwDict& dict, Scanner& scanner);

        // external dictionary, external decoder
        explicit CmfLoader(LzwDecoder& decoder);

        // no dictionary, no decoder
        explicit CmfLoader(Scanner& scanner);

        ~CmfLoader();

        void loadMolecule(Molecule& mol);
        void loadXyz(Scanner& scanner);

        bool skip_cistrans;
        bool skip_stereocenters;
        bool skip_valence;

        int version; // By default the latest version 2 is used

        Array<int>* atom_flags;
        Array<int>* bond_flags;

        bool has_mapping;
        CP_DECL;
        TL_CP_DECL(Array<int>, atom_mapping_to_restore);
        TL_CP_DECL(Array<int>, inv_atom_mapping_to_restore);
        TL_CP_DECL(Array<int>, bond_mapping_to_restore);
        TL_CP_DECL(Array<int>, inv_bond_mapping_to_restore);

        DECL_ERROR;

    protected:
        struct _AtomDesc
        {
            int label;
            int pseudo_atom_idx; // refers to _pseudo_labels
            int isotope;
            int charge;
            int hydrogens;
            int valence;
            int radical;

            int stereo_type;
            int stereo_group;
            bool stereo_invert_pyramid;

            int allene_stereo_parity;

            int flags;

            bool rsite;
            int rsite_bits;

            bool highlighted;
        };

        struct _BondDesc
        {
            int beg;
            int end;
            int type;
            int cis_trans;
            bool in_ring;
            int direction;
            bool swap;

            int flags;

            bool highlighted;
        };

        struct _AttachmentDesc
        {
            int atom;
            int index;
        };

        void _init();

        bool _getNextCode(int& code);

        void _readBond(int& code, _BondDesc& bond);
        bool _readAtom(int& code, _AtomDesc& atom, int atom_idx);
        bool _readCycleNumber(int& code, int& n);

        void _readExtSection(Molecule& mol);
        void _readSGroup(int code, Molecule& mol);
        void _readGeneralSGroup(SGroup& sgroup);

        void _readSGroupXYZ(Scanner& scanner, int code, Molecule& mol, const CmfSaver::VecRange& range);
        void _readBaseSGroupXyz(Scanner& scanner, SGroup& sgroup, const CmfSaver::VecRange& range);

        void _readString(Array<char>& dest);
        void _readUIntArray(Array<int>& dest);

        void _readVec3f(Scanner& scanner, Vec3f& pos, const CmfSaver::VecRange& range);
        void _readVec2f(Scanner& scanner, Vec2f& pos, const CmfSaver::VecRange& range);
        void _readDir2f(Scanner& scanner, Vec2f& dir, const CmfSaver::VecRange& range);
        float _readFloatInRange(Scanner& scanner, float min, float range);

        Scanner* _scanner;

        Obj<LzwDecoder> _decoder_obj;
        LzwDecoder* _ext_decoder;
        Obj<LzwScanner> _lzw_scanner;

        TL_CP_DECL(Array<_AtomDesc>, _atoms);
        TL_CP_DECL(Array<_BondDesc>, _bonds);
        TL_CP_DECL(StringPool, _pseudo_labels);
        TL_CP_DECL(Array<_AttachmentDesc>, _attachments);
        TL_CP_DECL(Array<int>, _sgroup_order);
        Molecule* _mol;

    private:
        CmfLoader(const CmfLoader&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif /* __cmf_loader_h__ */
