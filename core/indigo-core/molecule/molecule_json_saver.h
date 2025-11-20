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

#ifndef __molecule_json_saver_h__
#define __molecule_json_saver_h__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <sstream>

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/json_writer.h"
#include "molecule/meta_commons.h"
#include "molecule/monomers_lib.h"
#include "molecule/query_molecule.h"

namespace indigo
{

    class Molecule;
    class QueryMolecule;
    class Output;
    class MonomerTemplate;
    class ReactionMultistepDetector;

    class DLLEXPORT MoleculeJsonSaver
    {
    public:
        explicit MoleculeJsonSaver(Output& output);
        explicit MoleculeJsonSaver(Output& output, ReactionMultistepDetector& rmd);
        void saveMolecule(BaseMolecule& bmol);
        void saveMolecule(BaseMolecule& bmol, IJsonWriter& writer);
        void saveMetaData(IJsonWriter& writer, const MetaDataStorage& meta);
        void saveRoot(BaseMolecule& mol, IJsonWriter& writer);
        void saveMonomerTemplate(TGroup& tg, IJsonWriter& writer);

        static void parseFormatMode(const char* version_str, KETVersion& version);
        static void saveFormatMode(KETVersion& version, Array<char>& output);

        static void saveTextV1(IJsonWriter& writer, const SimpleTextObject& text_obj);
        static void saveTextV2(IJsonWriter& writer, const SimpleTextObject& text_obj);
        static void saveAlignment(IJsonWriter& writer, SimpleTextObject::TextAlignment alignment);
        static void saveFontStyles(IJsonWriter& writer, const FONT_STYLE_SET& fss);
        static void saveParagraphs(IJsonWriter& writer, const SimpleTextObject& text_obj);
        static void saveParts(IJsonWriter& writer, const SimpleTextObject::KETTextParagraph& paragraph, const FONT_STYLE_SET& def_fss);

        bool add_stereo_desc;
        bool add_reaction_data;
        bool pretty_json;
        bool use_native_precision; // TODO: Remove option and use_native_precision always - have to fix a lot of UTs
        KETVersion ket_version;

    protected:
        void saveMoleculeReference(int mol_id, IJsonWriter& writer);
        void saveEndpoint(BaseMolecule& mol, const std::string& ep, int beg_idx, int end_idx, IJsonWriter& writer, bool hydrogen = false);
        int getMonomerNumber(int mon_idx);

        void writeFloat(IJsonWriter& writer, float f_value);
        void writePos(IJsonWriter& writer, const Vec3f& pos);

        void saveAtoms(BaseMolecule& mol, IJsonWriter& writer);
        void saveBonds(BaseMolecule& mol, IJsonWriter& writer);
        void saveRGroup(RGroup& rgroup, int rgnum, IJsonWriter& writer);
        void saveFragment(BaseMolecule& fragment, IJsonWriter& writer);
        void saveAmbiguousMonomerTemplate(TGroup& tg, IJsonWriter& writer);
        void saveMonomerAttachmentPoints(TGroup& tg, IJsonWriter& writer);
        void saveSuperatomAttachmentPoints(Superatom& sa, IJsonWriter& writer);

        void saveSGroups(BaseMolecule& mol, IJsonWriter& writer);
        void saveSGroup(SGroup& sgroup, IJsonWriter& writer);

        void saveAttachmentPoint(BaseMolecule& mol, int atom_idx, IJsonWriter& writer);
        void saveStereoCenter(BaseMolecule& mol, int atom_idx, IJsonWriter& writer);
        void saveHighlights(BaseMolecule& mol, IJsonWriter& writer);

        void saveAnnotation(IJsonWriter& writer, const KetObjectAnnotation& annotation);

        DECL_ERROR;

    protected:
        void _checkSGroupIndices(BaseMolecule& mol, Array<int>& sgs_list);
        bool _checkAttPointOrder(BaseMolecule& mol, int rsite);
        bool _needCustomQuery(QueryMolecule::Atom* atom) const;
        void _writeQueryProperties(QueryMolecule::Atom* atom, IJsonWriter& writer);

        Molecule* _pmol;
        QueryMolecule* _pqmol;
        Output& _output;
        std::list<std::unordered_set<int>> _s_neighbors;
        std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash> _templates;
        std::unordered_map<std::pair<int, int>, std::string, pair_int_hash> _monomer_connections;
        std::map<int, int> _monomers_enum;
        std::vector<std::unique_ptr<BaseMolecule>> _no_template_molecules;
        ObjArray<Array<int>> _mappings;
        std::unordered_map<int, int> _atom_to_mol_id;
        std::optional<std::reference_wrapper<ReactionMultistepDetector>> _rmd;

    private:
        MoleculeJsonSaver(const MoleculeJsonSaver&); // no implicit copy
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
