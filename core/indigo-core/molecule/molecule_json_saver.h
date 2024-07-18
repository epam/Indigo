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

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/ket_commons.h"
#include "molecule/monomers_lib.h"
#include "molecule/query_molecule.h"

namespace indigo
{

    class Molecule;
    class QueryMolecule;
    class Output;

    class DLLEXPORT JsonWriter
    {
    public:
        typedef rapidjson::Writer<rapidjson::StringBuffer>::Ch Ch;

        explicit JsonWriter(bool is_pretty = false) : pretty_json(is_pretty)
        {
        }

        void Reset(rapidjson::StringBuffer& os)
        {
            if (pretty_json)
                _pretty_writer.Reset(os);
            else
                _writer.Reset(os);
        }

        bool IsComplete() const
        {
            return pretty_json ? _pretty_writer.IsComplete() : _writer.IsComplete();
        }

        int GetMaxDecimalPlaces() const
        {
            return pretty_json ? _pretty_writer.GetMaxDecimalPlaces() : _writer.GetMaxDecimalPlaces();
        }

        void SetMaxDecimalPlaces(int maxDecimalPlaces)
        {
            if (pretty_json)
                _pretty_writer.SetMaxDecimalPlaces(maxDecimalPlaces);
            else
                _writer.SetMaxDecimalPlaces(maxDecimalPlaces);
        }

        bool Null()
        {
            return pretty_json ? _pretty_writer.Null() : _writer.Null();
        }

        bool Bool(bool b)
        {
            return pretty_json ? _pretty_writer.Bool(b) : _writer.Bool(b);
        }

        bool Int(int i)
        {
            return pretty_json ? _pretty_writer.Int(i) : _writer.Int(i);
        }

        bool Uint(unsigned u)
        {
            return pretty_json ? _pretty_writer.Uint(u) : _writer.Uint(u);
        }

        bool Int64(int64_t i64)
        {
            return pretty_json ? _pretty_writer.Uint64(i64) : _writer.Uint64(i64);
        }

        bool Uint64(uint64_t u64)
        {
            return pretty_json ? _pretty_writer.Uint64(u64) : _writer.Uint64(u64);
        }

        bool Double(double d)
        {
            return pretty_json ? _pretty_writer.Double(d) : _writer.Double(d);
        }

        bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false)
        {
            return pretty_json ? _pretty_writer.RawNumber(str, length, copy) : _writer.RawNumber(str, length, copy);
        }

        bool String(const Ch* str, rapidjson::SizeType length, bool copy = false)
        {
            return pretty_json ? _pretty_writer.String(str, length, copy) : _writer.String(str, length, copy);
        }

        bool StartObject()
        {
            return pretty_json ? _pretty_writer.StartObject() : _writer.StartObject();
        }

        bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false)
        {
            return pretty_json ? _pretty_writer.Key(str, length, copy) : _writer.Key(str, length, copy);
        }

        bool EndObject(rapidjson::SizeType memberCount = 0)
        {
            return pretty_json ? _pretty_writer.EndObject(memberCount) : _writer.EndObject(memberCount);
        }

        bool StartArray()
        {
            return pretty_json ? _pretty_writer.StartArray() : _writer.StartArray();
        }

        bool EndArray(rapidjson::SizeType elementCount = 0)
        {
            return pretty_json ? _pretty_writer.EndArray(elementCount) : _writer.EndArray(elementCount);
        }

        bool String(const Ch* const& str)
        {
            return pretty_json ? _pretty_writer.String(str) : _writer.String(str);
        }

        bool String(const std::string& str)
        {
            return String(str.c_str());
        }

        bool Key(const Ch* const& str)
        {
            return pretty_json ? _pretty_writer.Key(str) : _writer.Key(str);
        }

        bool RawValue(const Ch* json, size_t length, rapidjson::Type type)
        {
            return pretty_json ? _pretty_writer.RawValue(json, length, type) : _writer.RawValue(json, length, type);
        }

        void Flush()
        {
            if (pretty_json)
                _pretty_writer.Flush();
            else
                _writer.Flush();
        }

        void WritePoint(const Vec2f& point)
        {
            if (pretty_json)
            {
                _pretty_writer.StartObject();
                _pretty_writer.Key("x");
                _pretty_writer.Double(point.x);
                _pretty_writer.Key("y");
                _pretty_writer.Double(point.y);
                _pretty_writer.Key("z");
                _pretty_writer.Double(0.0);
                _pretty_writer.EndObject(); // end position
            }
            else
            {
                _writer.StartObject();
                _writer.Key("x");
                _writer.Double(point.x);
                _writer.Key("y");
                _writer.Double(point.y);
                _writer.Key("z");
                _writer.Double(0.0);
                _writer.EndObject(); // end position
            }
        }

        void WriteRect(const Rect2f& rect)
        {
            if (pretty_json)
            {
                _pretty_writer.StartObject();
                _pretty_writer.Key("x");
                _pretty_writer.Double(rect.left());
                _pretty_writer.Key("y");
                _pretty_writer.Double(rect.top());
                _pretty_writer.Key("width");
                _pretty_writer.Double(rect.width());
                _pretty_writer.Key("height");
                _pretty_writer.Double(rect.height());
                _pretty_writer.EndObject();
            }
            else
            {
                _writer.StartObject();
                _writer.Key("x");
                _writer.Double(rect.left());
                _writer.Key("y");
                _writer.Double(rect.top());
                _writer.Key("width");
                _writer.Double(rect.width());
                _writer.Key("height");
                _writer.Double(rect.height());
                _writer.EndObject();
            }
        }

    private:
        bool pretty_json;
        rapidjson::Writer<rapidjson::StringBuffer> _writer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> _pretty_writer;
    };

    class DLLEXPORT MoleculeJsonSaver
    {
    public:
        explicit MoleculeJsonSaver(Output& output);
        void saveMolecule(BaseMolecule& bmol);
        void saveMolecule(BaseMolecule& bmol, JsonWriter& writer);
        void saveMetaData(JsonWriter& writer, const MetaDataStorage& meta);

        static void parseFormatMode(const char* version_str, KETVersion& version);
        static void saveFormatMode(KETVersion& version, Array<char>& output);

        static void saveTextV1(JsonWriter& writer, const KETTextObject& text_obj);
        static void saveText(JsonWriter& writer, const KETTextObject& text_obj);
        static void saveAlignment(JsonWriter& writer, KETTextObject::TextAlignment alignment);
        static void saveIndent(JsonWriter& writer, const KETTextObject::KETTextIndent& indent);
        static void saveFontStyles(JsonWriter& writer, const FONT_STYLE_SET& fss);
        static void saveParagraphs(JsonWriter& writer, const std::list<KETTextObject::KETTextParagraph>& paragraphs);
        static void saveParts(JsonWriter& writer, const std::map<std::size_t, FONT_STYLE_SET>& fss_map, const std::string& text);

        static std::string monomerId(const TGroup& tg);
        static std::string monomerKETClass(const std::string& class_name);
        static std::string monomerHELMClass(const std::string& class_name);

        bool add_stereo_desc;
        bool pretty_json;
        bool use_native_precision; // TODO: Remove option and use_native_precision always - have to fix a lot of UTs
        KETVersion ket_version;

    protected:
        void saveRoot(BaseMolecule& mol, JsonWriter& writer);
        void saveMoleculeReference(int mol_id, JsonWriter& writer);
        void saveEndpoint(BaseMolecule& mol, const std::string& ep, int beg_idx, int end_idx, JsonWriter& writer);
        int getMonomerNumber(int mon_idx);

        void writeFloat(JsonWriter& writer, float f_value);
        void saveAtoms(BaseMolecule& mol, JsonWriter& writer);
        void saveBonds(BaseMolecule& mol, JsonWriter& writer);
        void saveRGroup(PtrPool<BaseMolecule>& fragments, int rgnum, JsonWriter& writer);
        void saveFragment(BaseMolecule& fragment, JsonWriter& writer);
        void saveMonomerTemplate(TGroup& tg, JsonWriter& writer);
        void saveMonomerAttachmentPoints(TGroup& tg, JsonWriter& writer);
        void saveSuperatomAttachmentPoints(Superatom& sa, JsonWriter& writer);

        void saveSGroups(BaseMolecule& mol, JsonWriter& writer);
        void saveSGroup(SGroup& sgroup, JsonWriter& writer);

        void saveAttachmentPoint(BaseMolecule& mol, int atom_idx, JsonWriter& writer);
        void saveStereoCenter(BaseMolecule& mol, int atom_idx, JsonWriter& writer);
        void saveHighlights(BaseMolecule& mol, JsonWriter& writer);

        DECL_ERROR;

    protected:
        void _checkSGroupIndices(BaseMolecule& mol, Array<int>& sgs_list);
        bool _checkAttPointOrder(BaseMolecule& mol, int rsite);
        bool _needCustomQuery(QueryMolecule::Atom* atom) const;
        void _writeQueryProperties(QueryMolecule::Atom* atom, JsonWriter& writer);

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

    private:
        MoleculeJsonSaver(const MoleculeJsonSaver&); // no implicit copy
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
