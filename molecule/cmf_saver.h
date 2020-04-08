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

#ifndef __cmf_saver_h__
#define __cmf_saver_h__

#include "base_cpp/obj.h"
#include "lzw/lzw_encoder.h"
#include "math/algebra.h"
#include "molecule/base_molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Output;

    class DLLEXPORT CmfSaver
    {
    public:
        // external dictionary, internal encoder
        explicit CmfSaver(LzwDict& dict, Output& output);

        // external dictionary, external encoder
        explicit CmfSaver(LzwEncoder& encoder);

        // no dictionary, no encoder
        explicit CmfSaver(Output& output);

        void saveMolecule(Molecule& mol);
        void saveXyz(Output& output);

        const Array<int>& getAtomSequence();

        int* atom_flags;
        int* bond_flags;

        bool save_bond_dirs;
        bool save_highlighting;
        bool save_mapping;

        DECL_ERROR;

        struct VecRange
        {
            Vec3f xyz_min, xyz_range;
            bool have_z;
        };

    protected:
        void _init();
        void _encode(byte symbol);

        void _encodeAtom(Molecule& mol, int idx, const int* mapping);
        void _encodeBond(Molecule& mol, int idx, const int* mapping);
        void _encodeCycleNumer(int n);

        void _writeFloatInRange(Output& output, float v, float min, float range);

        struct Mapping
        {
            Array<int>*atom_mapping, *bond_mapping;
        };

        void _encodeString(const Array<char>& str);
        void _encodeUIntArray(const Array<int>& data, const Array<int>& mapping);
        void _encodeUIntArray(const Array<int>& data);
        void _encodeUIntArraySkipNegative(const Array<int>& data);

        void _encodeExtSection(Molecule& mol, const Mapping& mapping);
        void _encodeBaseSGroup(Molecule& mol, SGroup& sgroup, const Mapping& mapping);

        // void _encodeSGroups (Molecule &mol, const Mapping &mapping);
        void _writeSGroupsXyz(Molecule& mol, Output& output, const VecRange& range);
        void _writeBaseSGroupXyz(Output& output, SGroup& sgroup, const VecRange& range);

        void _writeVec3f(Output& output, const Vec3f& pos, const VecRange& range);
        void _writeVec2f(Output& output, const Vec2f& pos, const VecRange& range);
        void _writeDir2f(Output& output, const Vec2f& dir, const VecRange& range);

        void _updateSGroupsXyzMinMax(Molecule& mol, Vec3f& min, Vec3f& max);
        void _updateBaseSGroupXyzMinMax(SGroup& sgroup, Vec3f& min, Vec3f& max);

        CP_DECL;
        TL_CP_DECL(Array<int>, _atom_sequence);

        Output* _output;
        Obj<LzwEncoder> _encoder_obj;
        LzwEncoder* _ext_encoder;
        Obj<LzwOutput> _encoder_output_obj;

        Molecule* _mol;

    private:
        CmfSaver(const CmfSaver&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif /* __cmf_saver_h__ */
