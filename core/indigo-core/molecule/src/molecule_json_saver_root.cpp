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

#include <iomanip>
#include <memory>
#include <set>
#include <sstream>

#include "layout/molecule_layout.h"

#include "molecule/molecule.h"
#include "molecule/molecule_cip_calculator.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/molecule_savers.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/parse_utils.h"

#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"
#include "molecule/smiles_saver.h"
#include "reaction/pathway_reaction.h"
#include "reaction/reaction_multistep_detector.h"

#include <base_cpp/scanner.h>

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;
using namespace rapidjson;

void mergeMappings(Array<int>& dest, Array<int>& src)
{
    for (int i = 0; i < dest.size(); ++i)
    {
        int atom_idx = dest[i];
        if (atom_idx > -1 && atom_idx < src.size())
            dest[i] = src[atom_idx];
        else
            dest[i] = -1;
    }
}

static void saveNativeFloat(JsonWriter& writer, float f_value, int precision = -1)
{
    std::string val;
    if (precision >= 0)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << f_value;
        val = oss.str();
    }
    else
    {
        val = std::to_string(f_value);
    }
    writer.RawValue(val.c_str(), val.length(), kStringType);
}

void MoleculeJsonSaver::saveMonomerTemplate(TGroup& tg, JsonWriter& writer)
{
    std::string template_id("monomerTemplate-");
    std::string tg_id(monomerId(tg));
    std::string template_class(monomerKETClass(tg.tgroup_class.ptr()));
    std::string helm_class(monomerHELMClass(tg.tgroup_class.ptr()));
    template_id += tg_id;
    writer.Key(template_id.c_str());
    writer.StartObject();
    writer.Key("type");
    writer.String("monomerTemplate");
    writer.Key("id");
    writer.String(tg_id.c_str());
    if (tg.tgroup_class.size())
    {
        writer.Key("class");
        if (strcasecmp(template_class.c_str(), kMonomerClassLINKER) == 0)
            writer.String(kMonomerClassCHEM);
        else
            writer.String(template_class.c_str());
        writer.Key("classHELM");
        writer.String(helm_class.c_str());
    }

    writer.Key("alias");
    writer.String(monomerAlias(tg).c_str());

    if (tg.tgroup_name.size())
    {
        writer.Key("name");
        writer.String(tg.tgroup_name.ptr());
    }

    if (tg.tgroup_full_name.size())
    {
        writer.Key("fullName");
        writer.String(tg.tgroup_full_name.ptr());
    }

    std::string natreplace;
    if (tg.tgroup_natreplace.size() == 0)
    {
        auto alias = monomerAlias(tg);
        if (isBasicAminoAcid(template_class, alias))
        {
            natreplace = alias;
        }
        else if (tg.tgroup_name.size() > 0)
        {
            std::string name = tg.tgroup_name.ptr();
            alias = monomerAliasByName(tg.tgroup_class.ptr(), name);
            if (alias.size() > 0 && alias.size() != name.size())
                natreplace = alias;
        }
    }
    else
        natreplace = tg.tgroup_natreplace.ptr();

    if (natreplace.size())
    {
        auto analog = extractMonomerName(natreplace);
        auto nat_alias = monomerAliasByName(tg.tgroup_class.ptr(), analog);
        writer.Key("naturalAnalogShort");
        writer.String(nat_alias.c_str());
        if (analog.size() > 1)
        {
            writer.Key("naturalAnalog");
            writer.String(analog.c_str());
        }
    }

    if (tg.tgroup_comment.size())
    {
        writer.Key("comment");
        writer.String(tg.tgroup_comment.ptr());
    }

    if (tg.unresolved)
    {
        writer.Key("unresolved");
        writer.Bool(tg.unresolved);

        if (tg.idt_alias.size()) // Save IDT alias only for unresolved
        {
            writer.Key("idtAliases");
            writer.StartObject();
            writer.Key("base");
            writer.String(tg.idt_alias.ptr());
            writer.Key("modifications");
            writer.StartObject();
            writer.Key("endpoint5");
            writer.String(tg.idt_alias.ptr());
            writer.Key("internal");
            writer.String(tg.idt_alias.ptr());
            writer.Key("endpoint3");
            writer.String(tg.idt_alias.ptr());
            writer.EndObject();
            writer.EndObject();
        }
    }
    if (tg.modification_types.size() > 0)
    {
        writer.Key("modificationTypes");
        writer.StartArray();
        for (int i = 0; i < tg.modification_types.size(); i++)
        {
            writer.String(tg.modification_types[i].ptr());
        }
        writer.EndArray();
    }

    if (tg.different_aliasHELM)
    {
        writer.Key("aliasHELM");
        if (tg.aliasHELM.size() > 0)
            writer.String(tg.aliasHELM.ptr());
        else
            writer.String("");
    }

    if (tg.aliasAxoLabs.size() > 0)
    {
        writer.Key("aliasAxoLabs");
        writer.String(tg.aliasAxoLabs.ptr());
    }

    saveMonomerAttachmentPoints(tg, writer);
    saveFragment(*tg.fragment, writer);
    writer.EndObject();
}

void MoleculeJsonSaver::saveAmbiguousMonomerTemplate(TGroup& tg, JsonWriter& writer)
{
    std::string template_id("ambiguousMonomerTemplate-");
    std::string tg_id(monomerId(tg));
    std::string template_class(monomerKETClass(tg.tgroup_class.ptr()));
    std::string helm_class(monomerHELMClass(tg.tgroup_class.ptr()));
    template_id += tg_id;
    writer.Key(template_id.c_str());
    writer.StartObject();
    writer.Key("type");
    writer.String("ambiguousMonomerTemplate");
    writer.Key("subtype");
    writer.String(tg.mixture ? "mixture" : "alternatives");
    writer.Key("id");
    writer.String(tg_id.c_str());
    writer.Key("alias");
    writer.String(tg.tgroup_alias.ptr());
    writer.Key("options");
    writer.StartArray();
    const char* num_name = tg.mixture ? "ratio" : "probability";
    for (int i = 0; i < tg.aliases.size(); i++)
    {
        writer.StartObject();
        writer.Key("templateId");
        writer.String(tg.aliases[i].ptr());
        writer.EndObject();
        if (tg.ratios[i] >= 0)
        {
            writer.Key(num_name);
            saveNativeFloat(writer, tg.ratios[i], native_precision);
        }
    }
    writer.EndArray();
    writer.EndObject();
}

void MoleculeJsonSaver::saveSuperatomAttachmentPoints(Superatom& sa, JsonWriter& writer)
{
    std::map<std::string, int> sorted_attachment_points;
    if (sa.attachment_points.size())
    {
        for (int i = sa.attachment_points.begin(); i != sa.attachment_points.end(); i = sa.attachment_points.next(i))
        {
            auto& atp = sa.attachment_points[i];
            std::string atp_id_str(atp.apid.ptr());
            if (atp_id_str.size())
                sorted_attachment_points.insert(std::make_pair(atp_id_str, i));
        }

        if (sorted_attachment_points.size())
        {
            writer.Key("attachmentPoints");
            writer.StartArray();
            int order = 0;
            for (const auto& kvp : sorted_attachment_points)
            {
                writer.StartObject();
                auto& atp = sa.attachment_points[kvp.second];
                std::string atp_id_str(atp.apid.ptr());
                if (!isAttachmentPointsInOrder(order++, atp_id_str))
                {
                    if (atp_id_str.size())
                    {
                        writer.Key("id");
                        writer.String(atp_id_str.c_str());
                    }
                    writer.Key("type");
                    if (atp_id_str == kLeftAttachmentPoint || atp_id_str == kAttachmentPointR1)
                        writer.String("left");
                    else if (atp_id_str == kRightAttachmentPoint || atp_id_str == kAttachmentPointR2)
                        writer.String("right");
                    else
                        writer.String("side");
                    writer.Key("label");
                    writer.String(convertAPToHELM(atp_id_str).c_str());
                }
                writer.Key("attachmentAtom");
                writer.Int(atp.aidx);
                if (atp.lvidx >= 0)
                {
                    writer.Key("leavingGroup");
                    writer.StartObject();
                    writer.Key("atoms");
                    writer.StartArray();
                    writer.Int(atp.lvidx);
                    writer.EndArray();
                    writer.EndObject(); // leavingGroup
                }
                writer.EndObject(); // attachmentAtom
            }
            writer.EndArray();
        }
    }
}

void MoleculeJsonSaver::saveMonomerAttachmentPoints(TGroup& tg, JsonWriter& writer)
{
    auto& sgroups = tg.fragment->sgroups;
    for (int j = sgroups.begin(); j != sgroups.end(); j = sgroups.next(j))
    {
        SGroup& sg = sgroups.getSGroup(j);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            saveSuperatomAttachmentPoints((Superatom&)sg, writer);
            sgroups.remove(j);
        }
    }
}
void MoleculeJsonSaver::saveRGroup(RGroup& rgroup, int rgnum, JsonWriter& writer)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);

    if (rgroup.fragments.size() == 0 && rgroup.occurrence.size() == 0 && rgroup.if_then <= 0 && !rgroup.rest_h)
        return;

    buf.clear();
    out.printf("rg%d", rgnum);
    buf.push(0);

    writer.Key(buf.ptr());
    writer.StartObject();
    writer.Key("rlogic");
    writer.StartObject();
    writer.Key("number");
    writer.Int(rgnum);
    if (rgroup.occurrence.size() > 0)
    {
        buf.clear();
        rgroup.writeOccurrence(out);
        out.writeChar(0);
        writer.Key("range");
        writer.String(buf.ptr());
    }
    if (rgroup.if_then > 0)
    {
        writer.Key("ifthen");
        writer.Int(rgroup.if_then);
    }
    if (rgroup.rest_h)
    {
        writer.Key("resth");
        writer.Bool(rgroup.rest_h);
    }
    writer.EndObject(); // rlogic
    writer.Key("type");
    writer.String("rgroup");

    bool fmode = rgroup.fragments.size() > 1;
    if (fmode)
    {
        writer.Key("fragments");
        writer.StartArray();
    }

    for (int i = rgroup.fragments.begin(); i != rgroup.fragments.end(); i = rgroup.fragments.next(i))
    {
        if (fmode)
            writer.StartObject();
        saveFragment(*rgroup.fragments[i], writer);
        if (fmode)
            writer.EndObject();
    }

    if (fmode)
        writer.EndArray();

    writer.EndObject();
}

bool MoleculeJsonSaver::_checkAttPointOrder(BaseMolecule& mol, int rsite)
{
    const Vertex& vertex = mol.getVertex(rsite);
    for (int i = 0; i < vertex.degree() - 1; i++)
    {
        int cur = mol.getRSiteAttachmentPointByOrder(rsite, i);
        int next = mol.getRSiteAttachmentPointByOrder(rsite, i + 1);

        if (cur == VALUE_UNKNOWN || next == VALUE_UNKNOWN)
            return true; // here we treat "undefined" as "ok"

        if (cur > next)
            return false;
    }

    return true;
}

int MoleculeJsonSaver::getMonomerNumber(int mon_idx)
{
    auto mon_it = _monomers_enum.find(mon_idx);
    if (mon_it != _monomers_enum.end())
        return mon_it->second;
    else
        throw Error("Monomer index: %d not found", mon_idx);
    return -1;
}

void MoleculeJsonSaver::saveEndpoint(BaseMolecule& mol, const std::string& ep, int beg_idx, int end_idx, JsonWriter& writer, bool hydrogen)
{
    writer.Key(ep.c_str());
    writer.StartObject();
    if (mol.isTemplateAtom(beg_idx))
    {
        writer.Key("monomerId");
        writer.String((std::string("monomer") + std::to_string(getMonomerNumber(beg_idx))).c_str());
        auto conn_it = _monomer_connections.find(std::make_pair(beg_idx, end_idx));
        if (conn_it != _monomer_connections.end())
        {
            writer.Key("attachmentPointId");
            writer.String(convertAPToHELM(conn_it->second).c_str());
        }
        else if (!hydrogen) // Hydrogen connection has no attachment point
            throw Error("Attachment point not found!!!");
    }
    else
    {
        auto atom_mol_it = _atom_to_mol_id.find(beg_idx);
        if (atom_mol_it != _atom_to_mol_id.end())
        {
            int mol_id = atom_mol_it->second;
            writer.Key("moleculeId");
            writer.String((std::string("mol") + std::to_string(mol_id)).c_str());
            writer.Key("atomId");
            writer.String(std::to_string(_mappings[mol_id][beg_idx]).c_str());
        }
        else
            throw Error("Atom %d not found", beg_idx);
    }
    writer.EndObject();
}

void MoleculeJsonSaver::saveMoleculeReference(int mol_id, JsonWriter& writer)
{
    writer.StartObject();
    writer.Key("$ref");
    std::string mol_node = std::string("mol") + std::to_string(mol_id);
    writer.String(mol_node.c_str());
    writer.EndObject();
    auto& mapping = _mappings[mol_id];
    // printf("mol id:%d\n", mol_id);
    for (auto atom_idx = 0; atom_idx < mapping.size(); ++atom_idx)
    {
        if (mapping[atom_idx] > -1)
        {
            // printf("%d ", atom_idx);
            _atom_to_mol_id.emplace(atom_idx, mol_id);
        }
    }
    // printf("\n");
}

void MoleculeJsonSaver::saveAnnotation(JsonWriter& writer, const KetObjectAnnotation& annotation)
{
    writer.Key("annotation");
    writer.StartObject();
    annotation.saveOptsToKet(writer);
    writer.EndObject();
}

void MoleculeJsonSaver::saveRoot(BaseMolecule& mol, JsonWriter& writer)
{
    _no_template_molecules.clear();
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    writer.StartObject();

    // save KET version
    if (ket_version.major > KETVersion1.major)
    {
        writer.Key("ket_version");
        Array<char> version_str;
        saveFormatMode(ket_version, version_str);
        writer.String(version_str.ptr());
    }

    writer.Key("root");
    writer.StartObject();
    writer.Key("nodes");
    writer.StartArray();

    getSGroupAtoms(mol, _s_neighbors);
    // save mol references
    // int mol_id = 0;
    for (int idx = 0; idx < mol.countComponents(_s_neighbors); ++idx)
    {
        Filter filt(mol.getDecomposition().ptr(), Filter::EQ, idx);
        std::unique_ptr<BaseMolecule> component(mol.neu());
        Array<int> mapping, inv_mapping;
        component->makeSubmolecule(mol, filt, &mapping, &inv_mapping);
        if (!component->countTemplateAtoms())
        {
            _no_template_molecules.emplace_back(std::move(component));
            _mappings.push().copy(inv_mapping);
            saveMoleculeReference((int)_no_template_molecules.size() - 1, writer);
        }
        else
        {
            // collect non-template atoms
            Array<int> vertices;
            for (int atom_idx = component->vertexBegin(); atom_idx != component->vertexEnd(); atom_idx = component->vertexNext(atom_idx))
            {
                if (!component->isTemplateAtom(atom_idx))
                    vertices.push(atom_idx);
            }

            if (vertices.size())
            {
                Array<int> sub_mapping;
                std::unique_ptr<BaseMolecule> sub_mol(component->neu());
                sub_mol->makeSubmolecule(*component, vertices, &sub_mapping);
                mergeMappings(inv_mapping, sub_mapping);
                for (int sub_idx = 0; sub_idx < sub_mol->countComponents(); ++sub_idx)
                {
                    Array<int> sub_comp_mapping, mapping_cp, inv_sub_comp_mapping;
                    mapping_cp.copy(inv_mapping);
                    Filter filter(sub_mol->getDecomposition().ptr(), Filter::EQ, sub_idx);
                    std::unique_ptr<BaseMolecule> sub_mol_component(sub_mol->neu());
                    sub_mol_component->makeSubmolecule(*sub_mol, filter, &sub_comp_mapping, &inv_sub_comp_mapping);
                    _no_template_molecules.emplace_back(std::move(sub_mol_component));
                    mergeMappings(mapping_cp, inv_sub_comp_mapping);
                    _mappings.push().copy(mapping_cp);
                    saveMoleculeReference(static_cast<int>(_no_template_molecules.size()) - 1, writer);
                }
            }
        }
    }

    if (_rmd)
    {
        for (size_t i = 0; i < _rmd->get().reactionsInfo().size(); ++i)
        {
            writer.StartObject();
            writer.Key("$ref");
            std::string reaction_node = std::string("reaction") + std::to_string(i);
            writer.String(reaction_node.c_str());
            writer.EndObject();
        }
    }

    // save meta data
    saveMetaData(writer, mol.meta());

    // save rgroups
    for (int i = 1; i <= mol.rgroups.getRGroupCount(); ++i)
    {
        RGroup& rgroup = mol.rgroups.getRGroup(i);
        if (rgroup.fragments.size() == 0)
            continue;

        buf.clear();
        out.printf("rg%d", i);
        buf.push(0);
        writer.StartObject();
        writer.Key("$ref");
        writer.String(buf.ptr());
        writer.EndObject();
    }

    int mon_idx = 0;

    // save references to monomer's instances
    for (auto i : mol.vertices())
    {
        if (mol.isTemplateAtom(i))
        {
            writer.StartObject();
            writer.Key("$ref");
            writer.String((std::string("monomer") + std::to_string(mon_idx)).c_str());
            writer.EndObject();
            _monomers_enum.emplace(i, mon_idx++);
        }
    }

    // save references to monomer shapes
    for (int shape_idx = 0; shape_idx < mol.monomer_shapes.size(); ++shape_idx)
    {
        writer.StartObject();
        writer.Key("$ref");
        writer.String((KetMonomerShape::ref_prefix + std::to_string(shape_idx)).c_str());
        writer.EndObject();
    }

    writer.EndArray(); // nodes

    auto& annotation = mol.annotation();
    if (annotation.has_value())
    {
        writer.Key("annotation");
        writer.StartObject();
        annotation->saveOptsToKet(writer);
        auto& extended = annotation->extended();
        if (extended.has_value())
        {
            writer.Key("extended");
            extended->Accept(writer);
        }
        writer.EndObject();
    }

    // save connections and templates
    if (mol.tgroups.getTGroupCount())
    {
        // collect attachment points into unordered map <key, val>. key - pair of from and destination atom. val - attachment point name.
        _monomer_connections.clear();
        for (int i = mol.template_attachment_points.begin(); i != mol.template_attachment_points.end(); i = mol.template_attachment_points.next(i))
        {
            auto& sap = mol.template_attachment_points.at(i);
            _monomer_connections.emplace(std::make_pair(sap.ap_occur_idx, sap.ap_aidx), sap.ap_id.ptr());
        }

        // save connections
        writer.Key("connections");
        writer.StartArray();
        auto& bond_annotations = mol.getBondAnnotations();
        for (auto i : mol.edges())
        {
            auto& e = mol.getEdge(i);
            if (mol.isTemplateAtom(e.beg) || mol.isTemplateAtom(e.end))
            {
                // save connections between templates or atoms
                writer.StartObject();
                writer.Key("connectionType");
                bool hydrogen = mol.getBondOrder(i) == _BOND_HYDROGEN;
                writer.String(hydrogen ? "hydrogen" : "single");
                // save endpoints
                saveEndpoint(mol, "endpoint1", e.beg, e.end, writer, hydrogen);
                saveEndpoint(mol, "endpoint2", e.end, e.beg, writer, hydrogen);
                if (bond_annotations.count(i) > 0)
                    saveAnnotation(writer, bond_annotations.at(i));
                writer.EndObject(); // connection
            }
        }
        writer.EndArray(); // connections
        writer.Key("templates");
        writer.StartArray();

        for (int i = mol.tgroups.begin(); i != mol.tgroups.end(); i = mol.tgroups.next(i))
        {
            TGroup& tg = mol.tgroups.getTGroup(i);
            auto template_name = std::string(tg.ambiguous ? "ambiguousMonomerTemplate-" : "monomerTemplate-") + monomerId(tg);
            writer.StartObject();
            writer.Key("$ref");
            writer.String(template_name.c_str());
            writer.EndObject();
        }

        writer.EndArray(); // templates
    }
    writer.EndObject(); // root
}

void MoleculeJsonSaver::saveMolecule(BaseMolecule& bmol, JsonWriter& writer)
{
    if (add_stereo_desc)
        bmol.addCIP();

    std::unique_ptr<BaseMolecule> mol(bmol.neu());
    mol->clone_KeepIndices(bmol);

    if (!BaseMolecule::hasCoord(*mol))
    {
        MoleculeLayout ml(*mol, false);
        ml.layout_orientation = UNCPECIFIED;
        ml.make();
    }

    BaseMolecule::collapse(*mol);

    mol->getTemplatesMap(_templates);

    // save root elements
    saveRoot(*mol, writer);

    // save monomers
    if (mol->tgroups.getTGroupCount())
        for (auto i : mol->vertices())
        {
            if (mol->isTemplateAtom(i))
            {
                int mon_id = getMonomerNumber(i);
                writer.Key((std::string("monomer") + std::to_string(mon_id)).c_str());
                writer.StartObject();
                writer.Key("type");
                int temp_idx = mol->getTemplateAtomTemplateIndex(i);
                writer.String(temp_idx > -1 && bmol.tgroups.getTGroup(temp_idx).ambiguous ? "ambiguousMonomer" : "monomer");
                writer.Key("id");
                writer.String(std::to_string(mon_id).c_str());
                auto seqid = mol->getTemplateAtomSeqid(i);
                if (seqid != VALUE_UNKNOWN)
                {
                    writer.Key("seqid");
                    writer.Int(seqid);
                }
                // location
                writer.Key("position");
                const auto& pos = mol->getAtomXyz(i);
                writer.StartObject();
                writer.Key("x");
                writeFloat(writer, pos.x);
                writer.Key("y");
                writeFloat(writer, pos.y);
                writer.EndObject(); // pos

                auto display = mol->getTemplateAtomDisplayOption(i);
                if (display != DisplayOption::Undefined)
                {
                    writer.Key("expanded");
                    writer.Bool(display == DisplayOption::Expanded);
                }
                if (mol->isAtomSelected(i))
                {
                    writer.Key("selected");
                    writer.Bool(true);
                }

                auto transform = mol->getTemplateAtomTransform(i);
                if (transform.hasTransformation())
                {
                    writer.Key("transformation");
                    writer.StartObject();
                    if (transform.rotate != 0)
                    {
                        writer.Key("rotate");
                        writeFloat(writer, transform.rotate);
                    }
                    if (transform.shift.x != 0 || transform.shift.y != 0)
                    {
                        writer.Key("shift");
                        writer.StartObject();
                        writer.Key("x");
                        writeFloat(writer, transform.shift.x);
                        writer.Key("y");
                        writeFloat(writer, transform.shift.y);
                        writer.EndObject(); // shift
                    }
                    if (transform.flip != Transformation::FlipType::none)
                    {
                        writer.Key("flip");
                        writer.String(transform.getFlip());
                    }
                    writer.EndObject(); // transform
                }

                // find template
                writer.Key("alias");
                auto alias = mol->getTemplateAtom(i);
                writer.String(alias);
                auto mon_class = mol->getTemplateAtomClass(i);
                if (temp_idx > -1)
                {
                    auto& tg = bmol.tgroups.getTGroup(temp_idx);
                    writer.Key("templateId");
                    writer.String(monomerId(tg).c_str());
                }
                else
                {
                    auto tg_ref = findTemplateInMap(alias, mon_class, _templates);
                    if (tg_ref.has_value())
                    {
                        writer.Key("templateId");
                        writer.String(monomerId(tg_ref.value().get()).c_str());
                    }
                }
                if (mol->hasTemplateAtomAnnotation(i))
                    saveAnnotation(writer, mol->getTemplateAtomAnnotation(i));
                writer.EndObject(); // monomer
            }
        }

    // save templates
    for (int i = mol->tgroups.begin(); i != mol->tgroups.end(); i = mol->tgroups.next(i))
    {
        TGroup& tg = mol->tgroups.getTGroup(i);
        if (tg.ambiguous)
            saveAmbiguousMonomerTemplate(tg, writer);
        else
            saveMonomerTemplate(tg, writer);
    }

    // save molecules
    for (int i = 0; i < static_cast<int>(_no_template_molecules.size()); ++i)
    {
        auto& component = _no_template_molecules[i];
        if (component->vertexCount())
        {
            std::string mol_node = std::string("mol") + std::to_string(i);
            writer.Key(mol_node.c_str());
            writer.StartObject();
            writer.Key("type");
            writer.String("molecule");
            saveFragment(*component, writer);
            // TODO: the code below needs refactoring
            Vec3f flag_pos;
            if (bmol.getStereoFlagPosition(i, flag_pos))
            {
                writer.Key("stereoFlagPosition");
                writer.StartObject();
                writer.Key("x");
                writeFloat(writer, flag_pos.x);
                writer.Key("y");
                writeFloat(writer, flag_pos.y);
                writer.Key("z");
                writeFloat(writer, flag_pos.z);
                writer.EndObject();
            }
            writer.EndObject();
        }
    }

    // save R-Groups
    for (int i = 1; i <= mol->rgroups.getRGroupCount(); i++)
    {
        saveRGroup(mol->rgroups.getRGroup(i), i, writer);
    }

    // save reactions
    if (_rmd && add_reaction_data)
    {
        auto& reactions_info = _rmd->get().reactionsInfo();
        auto& summ_blocks = _rmd->get().summBlocks();
        auto& components = _rmd->get().molComponents();
        auto& complex_molecules_info = _rmd->get().complexMoleculesInfo();
        auto& special_conditions = _rmd->get().specialConditions();

        for (size_t i = 0; i < reactions_info.size(); ++i)
        {
            writer.Key((std::string("reaction") + std::to_string(i)).c_str());
            writer.StartObject();
            writer.Key("type");
            writer.String("reaction");

            bool has_groups = false;
            for (auto csb_idx : reactions_info[i].first)
            {
                auto& csb = summ_blocks[csb_idx];
                if (csb.indexes.size() > 1)
                {
                    if (!has_groups)
                    {
                        writer.Key("reactionGroups");
                        writer.StartArray();
                        has_groups = true;
                    }
                    writer.StartObject();
                    writer.Key("id");
                    writer.String((std::string("group") + std::to_string(csb_idx)).c_str());
                    writer.Key("components");
                    writer.StartArray();
                    for (auto& comp_idx : csb.indexes)
                    {
                        auto& mi = components[comp_idx].merged_indexes;
                        if (mi.size() > 1) // complex molecule
                        {
                            auto it_comp = complex_molecules_info.find(comp_idx);
                            if (it_comp != complex_molecules_info.end())
                                writer.String((std::string("complexMol") + std::to_string(it_comp->second.first)).c_str());
                        }
                        else if (mi.size() > 0) // single molecule
                            writer.String((std::string("mol") + std::to_string(mi.front())).c_str());
                    }
                    writer.EndArray();
                    if (csb.plus_indexes.size())
                    {
                        writer.Key("pluses");
                        writer.StartArray();
                        for (auto& plus_idx : csb.plus_indexes)
                            writer.String((std::string("plus") + std::to_string(plus_idx)).c_str());
                        writer.EndArray();
                    }
                    writer.EndObject();
                }
            }
            if (has_groups)
                writer.EndArray(); // participantGroups
            // collect steps
            writer.Key("steps");
            writer.StartArray();
            for (auto& kvp : reactions_info[i].second)
            {
                writer.StartObject();
                writer.Key("arrow");
                writer.String((std::string("arrow") + std::to_string(kvp.first)).c_str());
                std::vector<std::string> reactants, agents, products, conditions;
                std::string component_str;
                for (auto& pr : kvp.second)
                {
                    auto& csb = summ_blocks[pr.second];
                    if (csb.indexes.size() > 1)
                    {
                        component_str = (std::string("group") + std::to_string(pr.second));
                    }
                    else
                    {
                        auto& mi = components[csb.indexes.front()].merged_indexes;
                        if (mi.size() > 1) // complex molecule
                        {
                            auto it_comp = complex_molecules_info.find(csb.indexes.front());
                            if (it_comp != complex_molecules_info.end())
                                component_str = std::string("complexMol") + std::to_string(it_comp->second.first);
                        }
                        else if (mi.size() > 0) // single molecule
                            component_str = std::string("mol") + std::to_string(mi.front());
                    }
                    if (component_str.size())
                        switch (pr.first)
                        {
                        case BaseReaction::REACTANT:
                            reactants.push_back(component_str);
                            break;
                        case BaseReaction::PRODUCT:
                            products.push_back(component_str);
                            break;
                        case BaseReaction::CATALYST:
                            agents.push_back(component_str);
                            break;
                        }
                }
                if (reactants.size())
                {
                    writer.Key("reactants");
                    writer.StartArray();
                    for (auto& r : reactants)
                        writer.String(r.c_str());
                    writer.EndArray();
                }
                if (products.size())
                {
                    writer.Key("product");
                    writer.String(products.front().c_str());
                }
                if (agents.size())
                {
                    writer.Key("agents");
                    writer.StartArray();
                    for (auto& a : agents)
                        writer.String(a.c_str());
                    writer.EndArray();
                }
                auto spec_it = special_conditions.find(kvp.first);
                if (spec_it != special_conditions.end())
                {
                    auto& spec_cond = spec_it->second;
                    if (spec_cond.size())
                    {
                        writer.Key("conditions");
                        writer.StartArray();
                        for (auto& cond : spec_cond)
                            writer.String(std::string("text") + std::to_string(cond));
                        writer.EndArray();
                    }
                }

                writer.EndObject();
            }
            writer.EndArray();  // steps
            writer.EndObject(); // reaction
        }
    }

    // save monomer shapes
    for (int shape_idx = 0; shape_idx < mol->monomer_shapes.size(); ++shape_idx)
    {
        auto& monomer_shape = *mol->monomer_shapes[shape_idx];
        writer.Key((KetMonomerShape::ref_prefix + std::to_string(shape_idx)).c_str());
        writer.StartObject();
        writer.Key("type");
        writer.String("monomerShape");
        writer.Key("id");
        writer.String(monomer_shape.id());
        writer.Key("collapsed");
        writer.Bool(monomer_shape.collapsed());
        writer.Key("shape");
        writer.String(KetMonomerShape::shapeTypeToStr(monomer_shape.shape()).c_str());
        writer.Key("position");
        Vec2f pos = monomer_shape.position();
        writer.StartObject();
        writer.Key("x");
        saveNativeFloat(writer, pos.x, native_precision);
        writer.Key("y");
        saveNativeFloat(writer, pos.y, native_precision);
        writer.EndObject();
        writer.Key("monomers");
        writer.StartArray();
        for (auto& monomer_id : monomer_shape.monomers())
            writer.String(monomer_id);
        writer.EndArray();
        writer.EndObject();
    }

    writer.EndObject();
}

void MoleculeJsonSaver::saveFragment(BaseMolecule& fragment, JsonWriter& writer)
{
    _pmol = nullptr;
    _pqmol = nullptr;
    if (fragment.isQueryMolecule())
        _pqmol = &fragment.asQueryMolecule();
    else
        _pmol = &fragment.asMolecule();

    if (_pmol)
        _pmol->setIgnoreBadValenceFlag(true);

    writer.Key("atoms");
    writer.StartArray();
    saveAtoms(fragment, writer);
    writer.EndArray();

    writer.Key("bonds");
    writer.StartArray();
    saveBonds(fragment, writer);
    writer.EndArray();

    saveSGroups(fragment, writer);
    saveHighlights(fragment, writer);
    if (fragment.properties().size())
    {
        auto& props = fragment.properties().value(0);
        writer.Key("properties");
        writer.StartArray();
        for (auto it = props.elements().begin(); it != props.elements().end(); ++it)
        {
            writer.StartObject();
            writer.Key("key");
            writer.String(props.key(*it));
            writer.Key("value");
            writer.String(props.value(*it));
            writer.EndObject();
        }
        writer.EndArray();
    }
}

void MoleculeJsonSaver::saveMolecule(BaseMolecule& bmol)
{
    StringBuffer s;
    auto writer_ptr = JsonWriter::createJsonWriter(pretty_json);
    JsonWriter& writer = *writer_ptr;
    writer.Reset(s);
    saveMolecule(bmol, writer);
    std::stringstream result;
    result << s.GetString();
    _output.printf("%s", result.str().c_str());
}

void MoleculeJsonSaver::saveMetaData(JsonWriter& writer, const MetaDataStorage& meta)
{
    static const std::unordered_map<int, std::string> _arrow_type2string = {
        {ReactionComponent::ARROW_BASIC, "open-angle"},
        {ReactionComponent::ARROW_FILLED_TRIANGLE, "filled-triangle"},
        {ReactionComponent::ARROW_FILLED_BOW, "filled-bow"},
        {ReactionComponent::ARROW_DASHED, "dashed-open-angle"},
        {ReactionComponent::ARROW_FAILED, "failed"},
        {ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE, "both-ends-filled-triangle"},
        {ReactionComponent::ARROW_EQUILIBRIUM_FILLED_HALF_BOW, "equilibrium-filled-half-bow"},
        {ReactionComponent::ARROW_EQUILIBRIUM_FILLED_TRIANGLE, "equilibrium-filled-triangle"},
        {ReactionComponent::ARROW_EQUILIBRIUM_OPEN_ANGLE, "equilibrium-open-angle"},
        {ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_BOW, "unbalanced-equilibrium-filled-half-bow"},
        {ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_LARGE_FILLED_HALF_BOW, "unbalanced-equilibrium-large-filled-half-bow"},
        {ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE, "unbalanced-equilibrium-filled-half-triangle"},
        {ReactionComponent::ARROW_RETROSYNTHETIC, "retrosynthetic"}};

    const auto& meta_objects = meta.metaData();
    int arrow_id = 0, plus_id = 0, text_id = 0, multi_arrow_id = meta.getMetaCount(ReactionArrowObject::CID);
    for (int meta_index = 0; meta_index < meta_objects.size(); ++meta_index)
    {
        auto pobj = meta_objects[meta_index];
        switch (pobj->_class_id)
        {
        case ReactionArrowObject::CID: {
            ReactionArrowObject& ar = (ReactionArrowObject&)(*pobj);
            writer.StartObject();
            if (add_reaction_data)
            {
                writer.Key("id");
                writer.String(std::string("arrow") + std::to_string(arrow_id++));
            }
            writer.Key("type");
            writer.String("arrow");
            writer.Key("data");
            writer.StartObject();
            // arrow mode
            writer.Key("mode");
            std::string arrow_mode = "open-angle";
            auto at_it = _arrow_type2string.find(ar.getArrowType());
            if (at_it != _arrow_type2string.end())
                arrow_mode = at_it->second.c_str();
            writer.String(arrow_mode.c_str());

            // arrow coordinates
            writer.Key("pos");
            writer.StartArray();
            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getTail().x);
            writer.Key("y");
            writeFloat(writer, ar.getTail().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getHead().x);
            writer.Key("y");
            writeFloat(writer, ar.getHead().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.EndArray();  // arrow coordinates
            writer.EndObject(); // end data
            writer.EndObject(); // end node
        }
        break;
        case ReactionMultitailArrowObject::CID: {
            ReactionMultitailArrowObject& ar = (ReactionMultitailArrowObject&)(*pobj);
            writer.StartObject();
            if (add_reaction_data)
            {
                writer.Key("id");
                writer.String(std::string("arrow") + std::to_string(multi_arrow_id++));
            }

            writer.Key("type");
            writer.String("multi-tailed-arrow");
            writer.Key("data");
            writer.StartObject();

            writer.Key("head");
            writer.StartObject();
            writer.Key("position");
            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getHead().x);
            writer.Key("y");
            writeFloat(writer, ar.getHead().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();
            writer.EndObject();

            writer.Key("spine");
            writer.StartObject();
            writer.Key("pos");
            writer.StartArray();

            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getSpineBegin().x);
            writer.Key("y");
            writeFloat(writer, ar.getSpineBegin().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getSpineEnd().x);
            writer.Key("y");
            writeFloat(writer, ar.getSpineEnd().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.EndArray();
            writer.EndObject();

            writer.Key("tails");
            writer.StartObject();
            writer.Key("pos");
            writer.StartArray();

            for (auto& t : ar.getTails())
            {
                writer.StartObject();
                writer.Key("x");
                writeFloat(writer, t.x);
                writer.Key("y");
                writeFloat(writer, t.y);
                writer.Key("z");
                writer.Double(0);
                writer.EndObject();
            }

            writer.EndArray();
            writer.EndObject();

            writer.Key("zOrder");
            writer.Int(0);

            writer.EndObject();
            writer.EndObject();
        }
        break;
        case ReactionPlusObject::CID: {
            ReactionPlusObject& rp = (ReactionPlusObject&)(*pobj);
            writer.StartObject();
            if (add_reaction_data)
            {
                writer.Key("id");
                writer.String(std::string("plus") + std::to_string(plus_id++));
            }
            writer.Key("type");
            writer.String("plus");
            writer.Key("location");
            writer.StartArray();
            writeFloat(writer, rp.getPos().x);
            writeFloat(writer, rp.getPos().y);
            writer.Double(0);
            writer.EndArray();
            writer.EndObject();
        }
        break;
        case SimpleGraphicsObject::CID: {
            auto simple_obj = (SimpleGraphicsObject*)pobj;
            writer.StartObject();
            writer.Key("type");
            writer.String("simpleObject");
            writer.Key("data");
            writer.StartObject();
            writer.Key("mode");
            switch (simple_obj->_mode)
            {
            case SimpleGraphicsObject::EEllipse:
                writer.String("ellipse");
                break;
            case SimpleGraphicsObject::ERectangle:
                writer.String("rectangle");
                break;
            case SimpleGraphicsObject::ELine:
                writer.String("line");
                break;
            }
            writer.Key("pos");
            writer.StartArray();
            auto& coords = simple_obj->_coordinates;
            writer.WritePoint(coords.first);
            writer.WritePoint(coords.second);
            writer.EndArray();

            // end data
            writer.EndObject();
            // end node
            writer.EndObject();
            break;
        }
        case SimpleTextObject::CID: {
            auto ptext_obj = (SimpleTextObject*)pobj;
            writer.StartObject();
            if (add_reaction_data)
            {
                writer.Key("id");
                writer.String(std::string("text") + std::to_string(text_id++));
            }
            writer.Key("type");
            writer.String("text");
            if (ket_version.major == KETVersion1.major)
                saveTextV1(writer, *ptext_obj);
            else
                saveTextV2(writer, *ptext_obj);
            writer.EndObject(); // end node
            break;
        }
        case EmbeddedImageObject::CID: {
            auto image_obj = static_cast<const EmbeddedImageObject*>(pobj);
            auto& bbox = image_obj->getBoundingBox();
            writer.StartObject(); // start node
            writer.Key("type");
            writer.String("image");
            writer.Key("format");
            switch (image_obj->getFormat())
            {
            case EmbeddedImageObject::EKETPNG:
                writer.String(KImagePNG);
                break;
            case EmbeddedImageObject::EKETSVG:
                writer.String(KImageSVG);
                break;
            default:
                throw Exception("Bad image format: %d", image_obj->getFormat());
            }

            writer.Key("boundingBox");

            writer.StartObject(); // start bbox
            writer.Key("x");
            writeFloat(writer, bbox.left());
            writer.Key("y");
            writeFloat(writer, bbox.top());
            writer.Key("z");
            writer.Double(0);

            writer.Key("width");
            writeFloat(writer, bbox.width());
            writer.Key("height");
            writeFloat(writer, bbox.height());
            writer.EndObject(); // end bbox

            writer.Key("data");
            writer.String(image_obj->getBase64().c_str());
            writer.EndObject(); // end node
            break;
        }
        }
    }

    if (_rmd)
    {
        auto& complex_molecules_info = _rmd->get().complexMoleculesInfo();
        for (auto& cmol : complex_molecules_info)
        {
            writer.StartObject();
            writer.Key("id");
            std::string complex_mol_id = "complexMol" + std::to_string(cmol.second.first);
            writer.String(complex_mol_id.c_str());
            writer.Key("type");
            writer.String("complexMol");
            writer.Key("molecules");
            writer.StartArray();
            for (auto mol_idx : cmol.second.second)
            {
                std::string mol_id = "mol" + std::to_string(mol_idx);
                writer.String(mol_id.c_str());
            }
            writer.EndArray();
            writer.EndObject();
        }
    }
}

void MoleculeJsonSaver::saveTextV1(JsonWriter& writer, const SimpleTextObject& text_obj)
{
    std::string content = text_obj.content();
    if (content.empty() && text_obj.block().size())
    {
        // generate text from paragraphs
        SimpleTextObjectBuilder tob;
        auto default_fss = text_obj.fontStyles();
        for (const auto& paragraph : text_obj.block())
        {
            // merge font styles: default_fss + paragraph.font_style
            auto paragraph_fss = default_fss;
            paragraph_fss += paragraph.font_style;
            SimpleTextLine line;
            line.text = paragraph.text; // single part
            std::replace(line.text.begin(), line.text.end(), '\r', '\n');
            std::string_view text_view = std::string_view(line.text);
            KETFontStatusMap style_status_map;
            if (paragraph.font_styles.size() > 1)
                for (auto it_fss_kvp = paragraph.font_styles.rbegin(); it_fss_kvp != paragraph.font_styles.rend(); ++it_fss_kvp)
                {
                    auto current_part_fss = paragraph_fss;
                    auto prev_part_fss = paragraph_fss;

                    current_part_fss += it_fss_kvp->second; // current font state

                    auto it_fss_kvp_prev = it_fss_kvp != paragraph.font_styles.rbegin() ? std::prev(it_fss_kvp) : it_fss_kvp;

                    if (it_fss_kvp != it_fss_kvp_prev)
                        prev_part_fss += it_fss_kvp_prev->second; // previous font state

                    auto text_part = it_fss_kvp != it_fss_kvp_prev ? text_view.substr(it_fss_kvp->first, it_fss_kvp_prev->first - it_fss_kvp->first)
                                                                   : text_view.substr(it_fss_kvp->first);

                    for (auto& fss : current_part_fss)
                    {
                        auto fs = fss.first.getFontStyle();
                        auto fs_it = style_status_map.find(fs);
                        // check if font style is already in the map or values are different
                        if (fs_it == style_status_map.end())
                        {
                            // just add new font style with offset and size
                            style_status_map.emplace(
                                std::piecewise_construct, std::forward_as_tuple(fs),
                                std::forward_as_tuple(std::initializer_list<KETFontStyleStatus>{{it_fss_kvp->first, text_part.size(), fss.first.getVal()}}));
                        }
                        else // here we should update offset, size and val
                        {
                            // if val differs from previous or there is an offset gap
                            if (fs_it->second.front().val != fss.first.getVal() || fs_it->second.front().offset != it_fss_kvp_prev->first)
                                fs_it->second.emplace_front(it_fss_kvp->first, text_part.size(), fss.first.getVal());
                            else // extend font style
                            {
                                fs_it->second.front().offset = it_fss_kvp->first;
                                fs_it->second.front().size += text_part.size();
                            }
                        }
                    }
                }
            else
            {
                // add default font styles
            }
            if (style_status_map.size())
            {
                for (auto& [fs, ss_queue] : style_status_map)
                {
                    for (auto& ss : ss_queue)
                    {
                        SimpleTextStyle sts;
                        sts.offset = ss.offset;
                        sts.size = ss.size;
                        if (fs == KETFontStyle::FontStyle::ESize)
                        {
                            if (auto pval = std::get_if<uint32_t>(&ss.val))
                                sts.styles.push_back(std::string(KFontCustomSizeStrV1) + "_" + std::to_string(*pval) + "px");
                        }
                        else
                        {
                            auto it_fs = SimpleTextObject::textStyleMapInvV1().find(fs);
                            if (it_fs != SimpleTextObject::textStyleMapInvV1().end())
                                sts.styles.push_back(it_fs->second);
                        }
                        line.text_styles.push_back(sts);
                    }
                }
            }
            tob.addLine(line);
        }
        if (tob.getLineCounter())
            tob.finalize();
        content = tob.getJsonString();
    }

    writer.Key("data");
    writer.StartObject();
    writer.Key("content");
    writer.String(content.c_str());
    writer.Key("position");
    writer.WritePoint(text_obj.boundingBox().leftTop());
    writer.Key("pos");
    writer.StartArray();
    writer.WritePoint(text_obj.boundingBox().leftTop());
    writer.WritePoint(text_obj.boundingBox().leftBottom());
    writer.WritePoint(text_obj.boundingBox().rightBottom());
    writer.WritePoint(text_obj.boundingBox().rightTop());
    writer.EndArray();
    writer.EndObject();
}

void MoleculeJsonSaver::saveTextV2(JsonWriter& writer, const SimpleTextObject& text_obj)
{
    writer.Key("boundingBox");
    writer.WriteRect(text_obj.boundingBox());
    if (text_obj.alignment().has_value())
        saveAlignment(writer, text_obj.alignment().value());
    if (text_obj.indent().has_value())
    {
        writer.Key("indent");
        writer.Double(text_obj.indent().value());
    }
    if (text_obj.fontStyles().size())
        saveFontStyles(writer, text_obj.fontStyles());
    if (text_obj.block().size())
        saveParagraphs(writer, text_obj);
}

void MoleculeJsonSaver::saveFontStyles(JsonWriter& writer, const FONT_STYLE_SET& fss)
{
    std::vector<std::reference_wrapper<const std::pair<KETFontStyle, bool>>> font_fields;
    for (auto& fs : fss)
    {
        switch (fs.first.getFontStyle())
        {
        case KETFontStyle::FontStyle::EBold:
            writer.Key(KFontBoldStr);
            writer.Bool(fs.second);
            break;
        case KETFontStyle::FontStyle::EItalic:
            writer.Key(KFontItalicStr);
            writer.Bool(fs.second);
            break;
        case KETFontStyle::FontStyle::ESubScript:
            writer.Key(KFontSubscriptStr);
            writer.Bool(fs.second);
            break;
        case KETFontStyle::FontStyle::ESuperScript:
            writer.Key(KFontSuperscriptStr);
            writer.Bool(fs.second);
            break;
        case KETFontStyle::FontStyle::ENone:
            // default style
            break;
        default:
            if (fs.second)
                font_fields.push_back(std::ref(fs));
            break;
        }
    }

    if (font_fields.size())
    {
        writer.Key("font");
        writer.StartObject();
        for (auto& fs_ref : font_fields)
        {
            auto& fs_font = fs_ref.get();
            if (fs_font.second && fs_font.first.hasValue())
                switch (fs_font.first.getFontStyle())
                {
                case KETFontStyle::FontStyle::EColor: {
                    writer.Key(KFontColorStr);
                    std::stringstream ss;
                    ss << "#" << std::hex << fs_font.first.getUInt().value();
                    writer.String(ss.str());
                }
                break;
                case KETFontStyle::FontStyle::EFamily:
                    writer.Key(KFontFamilyStr);
                    writer.String(fs_font.first.getString().value());
                    break;
                case KETFontStyle::FontStyle::ESize:
                    writer.Key(KFontSizeStr);
                    writer.Uint(fs_font.first.getUInt().value());
                    break;
                }
        }
        writer.EndObject();
    }
}

void MoleculeJsonSaver::saveParagraphs(JsonWriter& writer, const SimpleTextObject& text_obj)
{
    const auto& paragraphs = text_obj.block();
    writer.Key("paragraphs");
    writer.StartArray();
    for (const auto& paragraph : paragraphs)
    {
        writer.StartObject();
        if (paragraph.alignment.has_value())
            saveAlignment(writer, paragraph.alignment.value());
        if (paragraph.indent.has_value())
        {
            writer.Key("indent");
            writer.Double(paragraph.indent.value());
        }

        auto def_fss = text_obj.fontStyles();
        def_fss &= paragraph.font_style;

        if (def_fss.size())
            saveFontStyles(writer, def_fss);

        if (paragraph.font_styles.size())
            saveParts(writer, paragraph, def_fss);
        if (paragraph.line_starts.has_value() && paragraph.line_starts.value().size())
        {
            writer.Key("lineStarts");
            writer.StartArray();
            for (auto ls : paragraph.line_starts.value())
                writer.Uint(ls);
            writer.EndArray();
        }
        writer.EndObject();
    }
    writer.EndArray();
}

void MoleculeJsonSaver::saveParts(JsonWriter& writer, const SimpleTextObject::KETTextParagraph& paragraph, const FONT_STYLE_SET& def_fss)
{
    if (paragraph.font_styles.size() > 1)
    {
        std::string_view text_view = std::string_view(paragraph.text);
        writer.Key("parts");
        writer.StartArray();
        for (auto it_fss_kvp = paragraph.font_styles.begin(); it_fss_kvp != std::prev(paragraph.font_styles.end()); ++it_fss_kvp)
        {
            writer.StartObject();
            auto next_it = std::next(it_fss_kvp);
            auto text_part = text_view.substr(it_fss_kvp->first, next_it->first - it_fss_kvp->first);
            writer.Key("text");
            writer.String(std::string(text_part).c_str());
            auto current_part_fss = def_fss;
            current_part_fss &= it_fss_kvp->second;
            if (current_part_fss.size())
                saveFontStyles(writer, current_part_fss);
            writer.EndObject();
        }
        writer.EndArray();
    }
}

void MoleculeJsonSaver::saveAlignment(JsonWriter& writer, SimpleTextObject::TextAlignment alignment)
{
    std::string alignment_str;
    switch (alignment)
    {
    case SimpleTextObject::TextAlignment::ELeft:
        alignment_str = KAlignmentLeft;
        break;
    case SimpleTextObject::TextAlignment::ERight:
        alignment_str = KAlignmentRight;
        break;
    case SimpleTextObject::TextAlignment::ECenter:
        alignment_str = KAlignmentCenter;
        break;
    case SimpleTextObject::TextAlignment::EFull:
        alignment_str = KAlignmentFull;
        break;
    }
    writer.Key("alignment");
    writer.String(alignment_str.c_str());
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
