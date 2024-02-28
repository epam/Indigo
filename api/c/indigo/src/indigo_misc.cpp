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

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "layout/molecule_layout.h"
#include "molecule/elements.h"
#include "molecule/icm_loader.h"
#include "molecule/icm_saver.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_hash.h"
#include "molecule/molecule_ionize.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/rdf_loader.h"
#include "molecule/sdf_loader.h"
#include "reaction/icr_loader.h"
#include "reaction/icr_saver.h"
#include "reaction/reaction_hash.h"
#include "reaction/reaction_json_saver.h"

#include "indigo_array.h"
#include "indigo_internal.h"
#include "indigo_loaders.h"
#include "indigo_molecule.h"
#include "indigo_properties.h"
#include "indigo_reaction.h"
#include "indigo_savers.h"
#include "indigo_structure_checker.h"

CEXPORT int indigoAromatize(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);
        AromaticityOptions arom_options = self.arom_options;
        arom_options.aromatize_skip_superatoms = self.aromatize_skip_superatoms;
        if (IndigoBaseMolecule::is(obj))
            return obj.getBaseMolecule().aromatize(arom_options) ? 1 : 0;
        if (IndigoBaseReaction::is(obj))
            return obj.getBaseReaction().aromatize(arom_options) ? 1 : 0;
        throw IndigoError("Only molecules and reactions can be aromatized");
    }
    INDIGO_END(-1);
}

CEXPORT int indigoDearomatize(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        AromaticityOptions arom_options = self.arom_options;
        arom_options.unique_dearomatization = self.unique_dearomatization;
        arom_options.aromatize_skip_superatoms = self.aromatize_skip_superatoms;

        if (IndigoBaseMolecule::is(obj))
            return obj.getBaseMolecule().dearomatize(arom_options) ? 1 : 0;
        if (IndigoBaseReaction::is(obj))
            return obj.getBaseReaction().dearomatize(arom_options) ? 1 : 0;
        throw IndigoError("Only molecules and reactions can be dearomatized");
    }
    INDIGO_END(-1);
}

#define INDIGO_SET_OPTION(SUFFIX, TYPE)                                                                                                                        \
    CEXPORT int indigoSetOption##SUFFIX(const char* name, TYPE value)                                                                                          \
    {                                                                                                                                                          \
        INDIGO_BEGIN                                                                                                                                           \
        {                                                                                                                                                      \
            sf::xlock_safe_ptr(indigoGetOptionManager())->callOptionHandler##SUFFIX(name, value);                                                              \
            return 1;                                                                                                                                          \
        }                                                                                                                                                      \
        INDIGO_END(-1);                                                                                                                                        \
    }

INDIGO_SET_OPTION(, const char*)
INDIGO_SET_OPTION(Int, int)
INDIGO_SET_OPTION(Bool, int)
INDIGO_SET_OPTION(Float, float)

CEXPORT int indigoSetOptionColor(const char* name, float r, float g, float b)
{
    INDIGO_BEGIN
    {
        sf::xlock_safe_ptr(indigoGetOptionManager())->callOptionHandlerColor(name, r, g, b);
        return 1;
    }
    INDIGO_END(-1);
}
CEXPORT int indigoSetOptionXY(const char* name, int x, int y)
{
    INDIGO_BEGIN
    {
        sf::xlock_safe_ptr(indigoGetOptionManager())->callOptionHandlerXY(name, x, y);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoGetOption(const char* name)
{
    INDIGO_BEGIN
    {
        auto& tmp = self.getThreadTmpData();
        sf::slock_safe_ptr(indigoGetOptionManager())->getOptionValueStr(name, tmp.string);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoGetOptionInt(const char* name, int* value)
{
    INDIGO_BEGIN
    {
        if (value)
        {
            sf::slock_safe_ptr(indigoGetOptionManager())->getOptionValueInt(name, *value);
            return 1;
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetOptionBool(const char* name, int* value)
{
    INDIGO_BEGIN
    {
        if (value)
        {
            sf::slock_safe_ptr(indigoGetOptionManager())->getOptionValueBool(name, *value);
            return 1;
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetOptionFloat(const char* name, float* value)
{
    INDIGO_BEGIN
    {
        if (value)
        {
            sf::slock_safe_ptr(indigoGetOptionManager())->getOptionValueFloat(name, *value);
            return 1;
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetOptionColor(const char* name, float* r, float* g, float* b)
{
    INDIGO_BEGIN
    {
        if (r && g && b)
        {
            sf::slock_safe_ptr(indigoGetOptionManager())->getOptionValueColor(name, *r, *g, *b);
            return 1;
        }
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetOptionXY(const char* name, int* x, int* y)
{
    INDIGO_BEGIN
    {
        if (x && y)
        {
            sf::slock_safe_ptr(indigoGetOptionManager())->getOptionValueXY(name, *x, *y);
            return 1;
        }
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoGetOptionType(const char* name)
{
    INDIGO_BEGIN
    {
        auto& tmp = self.getThreadTmpData();
        sf::slock_safe_ptr(indigoGetOptionManager())->getOptionType(name, tmp.string);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoResetOptions()
{
    INDIGO_BEGIN
    {
        auto mgr = sf::xlock_safe_ptr(indigoGetOptionManager());
        if (mgr->hasOptionHandler("reset-basic-options"))
        {
            mgr->callOptionHandlerVoid("reset-basic-options");
        }
        if (mgr->hasOptionHandler("reset-render-options"))
        {
            mgr->callOptionHandlerVoid("reset-render-options");
        }
        return 1;
    }
    INDIGO_END(-1);
}

void _indigoCheckBadValence(Molecule& mol)
{
    mol.restoreAromaticHydrogens();
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (mol.isPseudoAtom(i) || mol.isRSite(i))
            continue;
        mol.getAtomValence(i);
        mol.getImplicitH(i);
    }
}

CEXPORT const char* indigoCheckBadValence(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& bmol = obj.getBaseMolecule();

            if (bmol.isQueryMolecule())
                throw IndigoError("indigoCheckBadValence(): query molecules not allowed");

            Molecule& mol = bmol.asMolecule();

            try
            {
                _indigoCheckBadValence(mol);
            }
            catch (Exception& e)
            {
                auto& tmp = self.getThreadTmpData();
                tmp.string.readString(e.message(), true);
                return tmp.string.ptr();
            }
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& brxn = obj.getBaseReaction();

            if (brxn.isQueryReaction())
                throw IndigoError("indigoCheckBadValence(): query reactions not allowed");

            Reaction& rxn = brxn.asReaction();

            try
            {
                for (int j = rxn.begin(); j != rxn.end(); j = rxn.next(j))
                {
                    Molecule& mol = rxn.getMolecule(j);
                    _indigoCheckBadValence(mol);
                }
            }
            catch (Exception& e)
            {
                auto& tmp = self.getThreadTmpData();
                tmp.string.readString(e.message(), true);
                return tmp.string.ptr();
            }
        }
        else if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            if (ia.mol.isPseudoAtom(ia.idx) || ia.mol.isRSite(ia.idx) || ia.mol.isTemplateAtom(ia.idx))
                return "";

            try
            {
                int res = ia.mol.getAtomValence(ia.idx);
            }
            catch (Exception& e)
            {
                auto& tmp = self.getThreadTmpData();
                tmp.string.readString(e.message(), true);
                return tmp.string.ptr();
            }
        }
        else
            throw IndigoError("object %s is neither a molecule nor a reaction", obj.debugInfo());

        return "";
    }
    INDIGO_END(0);
}

void _indigoCheckAmbiguousH(Molecule& mol)
{
    mol.restoreAromaticHydrogens();
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        if (mol.getAtomAromaticity(i) == ATOM_AROMATIC)
        {
            int atom_number = mol.getAtomNumber(i);

            if (atom_number != ELEM_C && atom_number != ELEM_O)
                mol.getAtomTotalH(i);
        }
}

CEXPORT const char* indigoCheckAmbiguousH(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& bmol = obj.getBaseMolecule();

            if (bmol.isQueryMolecule())
                throw IndigoError("indigoCheckAmbiguousH(): query molecules not allowed");

            Molecule& mol = bmol.asMolecule();

            try
            {
                _indigoCheckAmbiguousH(mol);
            }
            catch (Exception& e)
            {
                auto& tmp = self.getThreadTmpData();
                tmp.string.readString(e.message(), true);
                return tmp.string.ptr();
            }
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& brxn = obj.getBaseReaction();

            if (brxn.isQueryReaction())
                throw IndigoError("indigoCheckAmbiguousH(): query molecules not allowed");

            Reaction& rxn = brxn.asReaction();

            try
            {
                int j;

                for (j = rxn.begin(); j != rxn.end(); j = rxn.next(j))
                    _indigoCheckAmbiguousH(rxn.getMolecule(j));
            }
            catch (Exception& e)
            {
                auto& tmp = self.getThreadTmpData();
                tmp.string.readString(e.message(), true);
                return tmp.string.ptr();
            }
        }
        else
            throw IndigoError("object %s is neither a molecule nor a reaction", obj.debugInfo());

        return "";
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoSmiles(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        auto& tmp = self.getThreadTmpData();
        IndigoSmilesSaver::generateSmiles(obj, tmp.string, self.smiles_saving_format);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoCanonicalSmiles(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        auto& tmp = self.getThreadTmpData();
        IndigoCanonicalSmilesSaver::generateSmiles(obj, tmp.string);

        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int64_t indigoHash(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        if (IndigoMolecule::is(obj))
        {
            Molecule& mol = obj.getMolecule();
            return static_cast<int64_t>(MoleculeHash::calculate(mol));
        }
        else if (IndigoReaction::is(obj))
        {
            Reaction& rxn = obj.getReaction();
            return static_cast<int64_t>(ReactionHash::calculate(rxn));
        }
        else
            throw IndigoError("object %s is neither a molecule nor a reaction", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoSmarts(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        auto& tmp = self.getThreadTmpData();
        IndigoSmilesSaver::generateSmarts(obj, tmp.string);

        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoCanonicalSmarts(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        auto& tmp = self.getThreadTmpData();
        IndigoCanonicalSmilesSaver::generateSmarts(obj, tmp.string);

        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

static void UnfoldAndLayoutHydrogens(BaseMolecule& bmol, bool only_selected, bool layout_hydrogens)
{
    Array<int> markers;
    bmol.unfoldHydrogens(&markers, -1, true, only_selected);
    if (layout_hydrogens && markers.count(1) > 0) // If some hydrogens found - layout it
    {
        // Layout hydrogens
        MoleculeLayoutGraphSmart layout;
        layout.preserve_existing_layout = true;
        layout.makeOnGraph(bmol);
        for (int i = layout.vertexBegin(); i < layout.vertexEnd(); i = layout.vertexNext(i))
        {
            const Vec3f& pos = bmol.getAtomXyz(layout.getVertexExtIdx(i));
            layout.getPos(i).set(pos.x, pos.y);
        }
        Filter new_filter(markers.ptr(), Filter::EQ, 1);
        layout.layout(bmol, 1, &new_filter, true);
        for (int i = layout.vertexBegin(); i < layout.vertexEnd(); i = layout.vertexNext(i))
        {
            const LayoutVertex& vert = layout.getLayoutVertex(i);
            bmol.setAtomXyz(vert.ext_idx, vert.pos.x, vert.pos.y, 0.f);
        }
    }
}

static bool MoleculeHasCoords(BaseMolecule& bmol)
{
    if (bmol.vertexCount() < 2) // If only one atom - treat as has coords
        return true;
    for (int i = bmol.vertexBegin(); i != bmol.vertexEnd(); i = bmol.vertexNext(i))
    {
        Vec3f& coords = bmol.getAtomXyz(i);
        if (coords.x != 0.0 || coords.y != 0.0 || coords.z != 0.0)
            return true;
    }
    return false;
}

CEXPORT int indigoUnfoldHydrogens(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        QS_DEF(Array<int>, markers);
        bool layout_hydrogens = false;

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& bmol = obj.getBaseMolecule();
            UnfoldAndLayoutHydrogens(bmol, bmol.countSelectedAtoms() > 0, MoleculeHasCoords(bmol));
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& rxn = obj.getBaseReaction();
            bool has_coords = false;
            bool only_selected = false;
            for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                if (MoleculeHasCoords(rxn.getBaseMolecule(i)))
                {
                    has_coords = true;
                    break;
                }

            for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                if (rxn.getBaseMolecule(i).countSelectedAtoms() > 0)
                {
                    only_selected = true;
                    break;
                }

            for (int i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
            {
                UnfoldAndLayoutHydrogens(rxn.getBaseMolecule(i), only_selected, has_coords);
            }
        }
        else
            throw IndigoError("indigoUnfoldHydrogens(): %s given", obj.debugInfo());

        return 1;
    }
    INDIGO_END(-1);
}

static bool hasConvertableToImplicitHydrogen(BaseMolecule& bmol, bool selected_only)
{
    for (int i = bmol.vertexBegin(); i != bmol.vertexEnd(); i = bmol.vertexNext(i))
    {
        if (bmol.convertableToImplicitHydrogen(i))
            if (!selected_only)
                return true;
            else if (bmol.isAtomSelected(i))
                return true;
    }
    return false;
}

CEXPORT int indigoFoldUnfoldHydrogens(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        bool selected_only = false;
        bool fold = false;
        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& bmol = obj.getBaseMolecule();
            if (bmol.countSelectedAtoms() > 0)
                selected_only = true;
            if (hasConvertableToImplicitHydrogen(bmol, selected_only))
                fold = true;
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& brxn = obj.getBaseReaction();
            for (int i = brxn.begin(); i != brxn.end(); i = brxn.next(i))
                if (brxn.getBaseMolecule(i).countSelectedAtoms() > 0)
                {
                    selected_only = true;
                    break;
                }
            for (int i = brxn.begin(); i != brxn.end(); i = brxn.next(i))
                if (hasConvertableToImplicitHydrogen(brxn.getBaseMolecule(i), selected_only))
                {
                    fold = true;
                    break;
                }
        }
        else
        {
            throw IndigoError("Unexpected object type.");
        }

        if (fold)
            return indigoFoldHydrogens(item);
        else
            return indigoUnfoldHydrogens(item);
    }
    INDIGO_END(-1);
}

static bool _removeHydrogens(BaseMolecule& mol, bool selected_only)
{
    QS_DEF(Array<int>, to_remove);
    QS_DEF(Array<int>, sterecenters_to_validate);
    int i;

    sterecenters_to_validate.clear();
    to_remove.clear();
    for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        if (!selected_only || mol.isAtomSelected(i))
            if (mol.convertableToImplicitHydrogen(i))
            {
                const Vertex& v = mol.getVertex(i);
                int nei = v.neiBegin();
                if (nei != v.neiEnd())
                {
                    if (mol.getBondDirection(v.neiEdge(nei)))
                        sterecenters_to_validate.push(v.neiVertex(nei));
                }
                to_remove.push(i);
            }

    if (to_remove.size() > 0)
        mol.removeAtoms(to_remove);
    for (int i = 0; i < sterecenters_to_validate.size(); i++)
        mol.markBondStereocenters(sterecenters_to_validate[i]);
    return to_remove.size() > 0;
}

CEXPORT int indigoFoldHydrogens(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& bmol = obj.getBaseMolecule();
            _removeHydrogens(bmol, bmol.countSelectedAtoms() > 0);
        }
        else if (IndigoBaseReaction::is(obj))
        {
            int i;
            BaseReaction& rxn = obj.getBaseReaction();
            bool selected_only = false;
            for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                if (rxn.getBaseMolecule(i).countSelectedAtoms() > 0)
                {
                    selected_only = true;
                    break;
                }

            for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
                _removeHydrogens(rxn.getBaseMolecule(i), selected_only);
        }
        else
            throw IndigoError("indigoFoldHydrogens(): %s given", obj.debugInfo());

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetName(int handle, const char* name)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (IndigoBaseMolecule::is(obj))
            obj.getBaseMolecule().name.readString(name, true);
        else if (IndigoBaseReaction::is(obj))
            obj.getBaseReaction().name.readString(name, true);
        else
            throw IndigoError("The object provided is neither a molecule, nor a reaction");
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoName(int handle)
{
    INDIGO_BEGIN
    {
        return self.getObject(handle).getName();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoRawData(int handler)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handler);

        auto& tmp = self.getThreadTmpData();

        if (obj.type == IndigoObject::RDF_MOLECULE || obj.type == IndigoObject::RDF_REACTION || obj.type == IndigoObject::SMILES_MOLECULE ||
            obj.type == IndigoObject::SMILES_REACTION || obj.type == IndigoObject::CML_MOLECULE || obj.type == IndigoObject::CML_REACTION ||
            obj.type == IndigoObject::CDX_MOLECULE || obj.type == IndigoObject::CDX_REACTION)
        {
            IndigoRdfData& data = (IndigoRdfData&)obj;

            tmp.string.copy(data.getRawData());
        }
        else if (obj.type == IndigoObject::PROPERTY)
            tmp.string.readString(((IndigoProperty&)obj).getValue(), false);
        else if (obj.type == IndigoObject::DATA_SGROUP)
        {
            tmp.string.copy(((IndigoDataSGroup&)obj).get().data);
        }
        else
            throw IndigoError("%s does not have raw data", obj.debugInfo());
        tmp.string.push(0);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoRemove(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        obj.remove();
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAt(int item, int index)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        if (obj.type == IndigoObject::SDF_LOADER)
        {
            IndigoObject* newobj = ((IndigoSdfLoader&)obj).at(index);
            if (newobj == 0)
                return 0;
            return self.addObject(newobj);
        }
        if (obj.type == IndigoObject::RDF_LOADER)
        {
            IndigoObject* newobj = ((IndigoRdfLoader&)obj).at(index);
            if (newobj == 0)
                return 0;
            return self.addObject(newobj);
        }
        else if (obj.type == IndigoObject::MULTILINE_SMILES_LOADER)
        {
            IndigoObject* newobj = ((IndigoMultilineSmilesLoader&)obj).at(index);
            if (newobj == 0)
                return 0;
            return self.addObject(newobj);
        }
        else if (obj.type == IndigoObject::MULTIPLE_CDX_LOADER)
        {
            IndigoObject* newobj = ((IndigoMultipleCdxLoader&)obj).at(index);
            if (newobj == 0)
                return 0;
            return self.addObject(newobj);
        }
        else if (IndigoArray::is(obj))
        {
            IndigoArray& arr = IndigoArray::cast(obj);

            return self.addObject(new IndigoArrayElement(arr, index));
        }
        else
            throw IndigoError("indigoAt(): not accepting %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCount(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoArray::is(obj))
            return IndigoArray::cast(obj).objects.size();

        if (obj.type == IndigoObject::SDF_LOADER)
            return ((IndigoSdfLoader&)obj).sdf_loader->count();

        if (obj.type == IndigoObject::RDF_LOADER)
            return ((IndigoRdfLoader&)obj).rdf_loader->count();

        if (obj.type == IndigoObject::MULTILINE_SMILES_LOADER)
            return ((IndigoMultilineSmilesLoader&)obj).count();

        throw IndigoError("indigoCount(): can not handle %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSerialize(int item, byte** buf, int* size)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        auto& tmp = self.getThreadTmpData();
        ArrayOutput out(tmp.string);

        if (IndigoBaseMolecule::is(obj))
        {
            Molecule& mol = obj.getMolecule();

            IcmSaver saver(out);
            saver.save_xyz = mol.have_xyz;
            saver.save_bond_dirs = true;
            saver.save_highlighting = true;
            saver.save_ordering = self.preserve_ordering_in_serialize;
            saver.saveMolecule(mol);
        }
        else if (IndigoBaseReaction::is(obj))
        {
            Reaction& rxn = obj.getReaction();
            IcrSaver saver(out);
            saver.save_xyz = BaseReaction::haveCoord(rxn);
            saver.save_bond_dirs = true;
            saver.save_highlighting = true;
            saver.save_ordering = self.preserve_ordering_in_serialize;
            saver.saveReaction(rxn);
        }

        *buf = (byte*)tmp.string.ptr();
        *size = tmp.string.size();
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoUnserialize(const byte* buf, int size)
{
    INDIGO_BEGIN
    {
        if (IcmSaver::checkVersion((const char*)buf))
        {
            BufferScanner scanner(buf, size);
            IcmLoader loader(scanner);
            std::unique_ptr<IndigoMolecule> im = std::make_unique<IndigoMolecule>();
            loader.loadMolecule(im->mol);
            return self.addObject(im.release());
        }
        else if (IcrSaver::checkVersion((const char*)buf))
        {
            BufferScanner scanner(buf, size);
            IcrLoader loader(scanner);
            std::unique_ptr<IndigoReaction> ir = std::make_unique<IndigoReaction>();
            loader.loadReaction(ir->rxn);
            return self.addObject(ir.release());
        }
        else
            throw IndigoError("indigoUnserialize(): format not recognized");
    }
    INDIGO_END(-1);
}

CEXPORT int indigoClear(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoArray::is(obj))
        {
            IndigoArray& array = IndigoArray::cast(obj);

            array.objects.clear();
        }
        else if (IndigoBaseMolecule::is(obj))
            obj.getBaseMolecule().clear();
        else if (IndigoBaseReaction::is(obj))
            obj.getBaseReaction().clear();
        else
            throw IndigoError("indigoClear(): do not know how to clear %s", obj.debugInfo());
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoHighlight(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            ia.mol.highlightAtom(ia.idx);
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(obj);

            ib.mol.highlightBond(ib.idx);
        }
        else
            throw IndigoError("indigoHighlight(): expected atom or bond, got %s", obj.debugInfo());

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoUnhighlight(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            ia.mol.unhighlightAtom(ia.idx);
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(obj);

            ib.mol.unhighlightBond(ib.idx);
        }
        else if (IndigoBaseMolecule::is(obj))
        {
            obj.getBaseMolecule().unhighlightAll();
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& reaction = obj.getBaseReaction();
            int i;

            for (i = reaction.begin(); i != reaction.end(); i = reaction.next(i))
                reaction.getBaseMolecule(i).unhighlightAll();
        }
        else
            throw IndigoError("indigoUnhighlight(): expected atom/bond/molecule/reaction, got %s", obj.debugInfo());

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIsHighlighted(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            return ia.mol.isAtomHighlighted(ia.idx) ? 1 : 0;
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(obj);

            return ib.mol.isBondHighlighted(ib.idx) ? 1 : 0;
        }
        else
            throw IndigoError("indigoIsHighlighted(): expected atom or bond, got %s", obj.debugInfo());

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSelect(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            ia.mol.selectAtom(ia.idx);
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(obj);

            ib.mol.selectBond(ib.idx);
        }
        else
            throw IndigoError("indigoSelect(): expected atom or bond, got %s", obj.debugInfo());

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoUnselect(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            ia.mol.unselectAtom(ia.idx);
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(obj);

            ib.mol.unselectBond(ib.idx);
        }
        else if (IndigoBaseMolecule::is(obj))
        {
            obj.getBaseMolecule().unselectAll();
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& reaction = obj.getBaseReaction();
            int i;

            for (i = reaction.begin(); i != reaction.end(); i = reaction.next(i))
                reaction.getBaseMolecule(i).unselectAll();
        }
        else
            throw IndigoError("indigoUnselect(): expected atom/bond/molecule/reaction, got %s", obj.debugInfo());

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIsSelected(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            return ia.mol.isAtomSelected(ia.idx) ? 1 : 0;
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(obj);

            return ib.mol.isBondSelected(ib.idx) ? 1 : 0;
        }
        else
            throw IndigoError("indigoIsSelected(): expected atom or bond, got %s", obj.debugInfo());

        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoOptimize(int query, const char* options)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(query);

        if (obj.type == IndigoObject::QUERY_MOLECULE)
        {
            IndigoQueryMolecule& qm_obj = (IndigoQueryMolecule&)obj;
            QueryMolecule& q = qm_obj.getQueryMolecule();
            q.optimize();

            QS_DEF(Array<int>, transposition);
            QS_DEF(QueryMolecule, transposed_q);
            qm_obj.getNeiCounters().makeTranspositionForSubstructure(q, transposition);
            transposed_q.makeSubmolecule(q, transposition, 0);
            q.clone(transposed_q, 0, 0);
        }
        else if (IndigoBaseReaction::is(obj))
            obj.getQueryReaction().optimize();
        else
            throw IndigoError("indigoOptimize: expected molecule or reaction, got %s", obj.debugInfo());
        return 1;
    }
    INDIGO_END(-1);
}

static int _indigoHasCoord(int item, bool (*has_coord_func)(BaseMolecule& mol), const char* func_name)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& mol = obj.getBaseMolecule();
            return has_coord_func(mol) ? 1 : 0;
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& reaction = obj.getBaseReaction();
            for (int i = reaction.begin(); i != reaction.end(); i = reaction.next(i))
            {
                BaseMolecule& mol = reaction.getBaseMolecule(i);
                if (has_coord_func(mol))
                    return 1;
            }
            return 0;
        }
        else
            throw IndigoError("%s: expected molecule or reaction, got %s", func_name, obj.debugInfo());
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoHasZCoord(int item)
{
    return _indigoHasCoord(item, BaseMolecule::hasZCoord, "indigoHasZCoord");
}

CEXPORT int indigoHasCoord(int item)
{
    return _indigoHasCoord(item, BaseMolecule::hasCoord, "indigoHasCoord");
}

CEXPORT const char* indigoDbgInternalType(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        char tmp_str[1024];
        snprintf(tmp_str, 1023, "#%02d: %s", obj.type, obj.debugInfo());
        auto& tmp = self.getThreadTmpData();
        tmp.string.readString(tmp_str, true);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT int indigoNormalize(int structure, const char* options)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(structure);
        Molecule& mol = obj.getMolecule();

        bool changed = false;

        // Fold hydrogens
        changed |= _removeHydrogens(mol, false);

        // Neutralize charges
        for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        {
            int charge = mol.getAtomCharge(i);
            if (charge == 1 && mol.getAtomNumber(i) == ELEM_N)
            {
                const Vertex& v = mol.getVertex(i);
                for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
                {
                    int j = v.neiVertex(nei);
                    int charge2 = mol.getAtomCharge(j);
                    if (charge2 == -1 && mol.getAtomNumber(j) == ELEM_O)
                    {
                        int edge_idx = v.neiEdge(nei);
                        if (mol.getBondOrder(edge_idx) == BOND_SINGLE)
                        {
                            mol.setAtomCharge(i, 0);
                            mol.setAtomCharge(j, 0);
                            mol.setBondOrder(edge_idx, BOND_DOUBLE);
                            changed = true;
                            break;
                        }
                    }
                }
            }
        }

        if (changed)
        {
            // Validate cs-trans because it can disappear
            // For example: [O-]/[N+](=C\C1C=CC=CC=1)/C1C=CC=CC=1
            mol.validateCisTrans();
        }

        return changed;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoStandardize(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        if (obj.type == IndigoObject::QUERY_MOLECULE)
        {
            IndigoQueryMolecule& qm_obj = (IndigoQueryMolecule&)obj;
            QueryMolecule& q = qm_obj.getQueryMolecule();
            q.standardize(self.standardize_options);
        }
        else if (obj.type == IndigoObject::MOLECULE)
        {
            IndigoMolecule& m_obj = (IndigoMolecule&)obj;
            Molecule& m = m_obj.getMolecule();
            m.standardize(self.standardize_options);
        }
        else
            throw IndigoError("indigoStandardize: expected molecule or query, got %s", obj.debugInfo());
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIonize(int object, float pH, float pH_toll)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);
        Molecule& mol = obj.getMolecule();
        mol.ionize(pH, pH_toll, self.ionize_options);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoBuildPkaModel(int max_level, float threshold, const char* filename)
{
    INDIGO_BEGIN
    {
        int level = MoleculePkaModel::buildPkaModel(max_level, threshold, filename);
        if (level > 0)
            return 1;
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT float* indigoGetAcidPkaValue(int object, int atom, int level, int min_level)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        if (obj.type == IndigoObject::MOLECULE)
        {
            IndigoMolecule& m_obj = (IndigoMolecule&)obj;
            Molecule& mol = m_obj.getMolecule();
            IndigoAtom& site = IndigoAtom::cast(self.getObject(atom));
            auto& tmp = self.getThreadTmpData();
            float pka = MoleculePkaModel::getAcidPkaValue(mol, site.getIndex(), level, min_level);
            tmp.xyz[0] = pka;
            return tmp.xyz;
        }
        else
            throw IndigoError("indigoGetAcidPkaValue: expected molecule, got %s", obj.debugInfo());
        return 0;
    }
    INDIGO_END(0);
}

CEXPORT float* indigoGetBasicPkaValue(int object, int atom, int level, int min_level)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        if (obj.type == IndigoObject::MOLECULE)
        {
            IndigoMolecule& m_obj = (IndigoMolecule&)obj;
            Molecule& mol = m_obj.getMolecule();
            IndigoAtom& site = IndigoAtom::cast(self.getObject(atom));
            auto& tmp = self.getThreadTmpData();
            float pka = MoleculePkaModel::getBasicPkaValue(mol, site.getIndex(), level, min_level);
            tmp.xyz[0] = pka;
            return tmp.xyz;
        }
        else
            throw IndigoError("indigoGetBasicPkaValue: expected molecule, got %s", obj.debugInfo());
        return 0;
    }
    INDIGO_END(0);
}

CEXPORT int indigoIsPossibleFischerProjection(int object, const char* options)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(object);

        if (obj.type == IndigoObject::MOLECULE)
        {
            IndigoMolecule& m_obj = (IndigoMolecule&)obj;
            Molecule& mol = m_obj.getMolecule();
            if (mol.isPossibleFischerProjection(options))
                return 1;
            return 0;
        }
        else
            throw IndigoError("indigoIsPossibleFischerProjection: expected molecule, got %s", obj.debugInfo());
        return -1;
    }
    INDIGO_END(-1);
}

void _parseHelmRgroupsNames(Array<char>& helm_caps, StringPool& r_names)
{
    BufferScanner strscan(helm_caps);
    QS_DEF(Array<char>, r_desc);
    QS_DEF(Array<char>, r_name);
    QS_DEF(Array<char>, delim);
    r_desc.clear();
    r_name.clear();
    delim.clear();
    r_names.clear();

    delim.push(',');
    delim.push(0);

    while (!strscan.isEOF())
    {
        strscan.readWord(r_desc, delim.ptr());
        if (strncmp(r_desc.ptr(), "[R", 2) == 0)
        {
            BufferScanner r_scan(r_desc.ptr());
            r_scan.skip(2);
            int rg_id = r_scan.readInt1();
            r_scan.readAll(r_name);
            while ((rg_id - 1) > r_names.size())
                r_names.add(1);
            r_names.add(r_name);
        }
        if (!strscan.isEOF())
            strscan.skip(1);
    }
}

CEXPORT int indigoTransformHELMtoSCSR(int object)
{
    INDIGO_BEGIN
    {
        QS_DEF(Array<char>, helm_class);
        QS_DEF(Array<char>, helm_name);
        QS_DEF(Array<char>, helm_code);
        QS_DEF(Array<char>, helm_natreplace);
        QS_DEF(Array<char>, helm_caps);
        QS_DEF(Array<char>, helm_type);
        QS_DEF(StringPool, r_names);

        IndigoObject& obj = self.getObject(object);

        if (obj.type == IndigoObject::RDF_MOLECULE)
        {
            std::unique_ptr<IndigoMolecule> im = std::make_unique<IndigoMolecule>();
            im->mol.clone(obj.getMolecule(), 0, 0);

            auto& props = obj.getProperties();

            if (props.contains("HELM_CLASS") && props.contains("HELM_NAME") && props.contains("HELM_CAPS"))
            {
                helm_class.readString(props.at("HELM_CLASS"), true);
                helm_name.readString(props.at("HELM_NAME"), true);
                helm_caps.readString(props.at("HELM_CAPS"), true);
            }
            else
                throw IndigoError("indigoTransformHELMtoSCSR: required properties not found.");

            if (props.contains("HELM_CODE"))
                helm_code.readString(props.at("HELM_CODE"), true);
            if (props.contains("HELM_NATREPLACE"))
                helm_natreplace.readString(props.at("HELM_NATREPLACE"), true);
            if (props.contains("HELM_TYPE"))
                helm_type.readString(props.at("HELM_TYPE"), true);

            _parseHelmRgroupsNames(helm_caps, r_names);

            im->mol.transformHELMtoSGroups(helm_class, helm_name, helm_code, helm_natreplace, r_names);

            return self.addObject(im.release());
        }
        else
            throw IndigoError("indigoTransformHELMtoSCSR: expected molecule, got %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCheckQuery(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoAtom::is(obj))
        {
            IndigoAtom& ia = IndigoAtom::cast(obj);

            if ((ia.mol.reaction_atom_exact_change[ia.idx] != 0) || (ia.mol.reaction_atom_inversion[ia.idx] != 0))
                return 1;

            if (ia.mol.isQueryMolecule())
            {
                return 1;
            }
        }
        else if (IndigoBond::is(obj))
        {
            IndigoBond& ib = IndigoBond::cast(obj);

            if (ib.mol.reaction_bond_reacting_center[ib.idx] != 0)
                return 1;

            if (ib.mol.isQueryMolecule())
            {
                return 1;
            }
        }
        else if (IndigoQueryMolecule::is(obj))
        {
            return 1;
        }
        else if (IndigoQueryReaction::is(obj))
        {
            return 1;
        }
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCheckChirality(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& bmol = obj.getBaseMolecule();
            int chiral_flag = bmol.getChiralFlag();
            if (chiral_flag == -1)
            {
                return 1;
            }
            else if ((chiral_flag > 0) && (bmol.stereocenters.size() == 0))
            {
                return 0;
            }
        }
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCheck3DStereo(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& bmol = obj.getBaseMolecule();

            bool stereo_3d = true;

            if ((bmol.stereocenters.size() > 0) && BaseMolecule::hasZCoord(bmol))
            {
                for (auto i : bmol.vertices())
                {
                    if (!bmol.stereocenters.exists(i))
                    {
                        const Vertex& vertex = bmol.getVertex(i);

                        for (auto j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
                            if (bmol.getBondDirection2(i, vertex.neiVertex(j)) > 0)
                                stereo_3d = false;
                    }
                }
            }
            else
                stereo_3d = false;

            if (stereo_3d)
                return 1;
        }
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCheckStereo(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);

        if (IndigoBaseMolecule::is(obj))
        {
            Molecule& mol = obj.getMolecule();
            QS_DEF(Molecule, target);
            target.clone_KeepIndices(mol);

            for (auto i : target.vertices())
            {
                if (!target.stereocenters.exists(i) && target.isPossibleStereocenter(i))
                {
                    target.addStereocenters(i, MoleculeStereocenters::ATOM_ABS, 0, false);
                }
            }

            MoleculeAutomorphismSearch as;

            as.detect_invalid_cistrans_bonds = true;
            as.detect_invalid_stereocenters = true;
            as.find_canonical_ordering = false;
            as.process(target);

            for (auto i : target.vertices())
            {
                if (target.stereocenters.exists(i) && as.invalidStereocenter(i))
                {
                    target.stereocenters.remove(i);
                }
            }

            if (mol.stereocenters.size() != target.stereocenters.size())
                return 1;
        }
        return 0;
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoCheck(const char* item, const char* check_flags, const char* load_params)
{
    INDIGO_BEGIN
    {
        self.stereochemistry_options.ignore_errors = true;
        auto& tmp = self.getThreadTmpData();
        IndigoStructureChecker checker;
        std::string r = checker.toJson(checker.check(item, check_flags, load_params));
        tmp.string.clear();
        tmp.string.appendString(r.c_str(), true);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoCheckObj(int item, const char* check_flags)
{
    INDIGO_BEGIN
    {
        auto& tmp = self.getThreadTmpData();
        IndigoStructureChecker checker;
        std::string r = checker.toJson(checker.check(item, check_flags));
        tmp.string.clear();
        tmp.string.appendString(r.c_str(), true);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoCheckStructure(const char* structure, const char* props)
{
    INDIGO_BEGIN
    {
        auto& tmp = self.getThreadTmpData();
        ArrayOutput out(tmp.string);

        try
        {
            int item = indigoLoadStructureFromString(structure, "");
            if (item > 0)
                out.writeString(indigoCheckObj(item, props));
            else
                out.writeString("{\"LOAD\":{\"message\":\"Error at loading structure\"}}");
        }
        catch (Exception& e)
        {
            out.printf("{\"LOAD\":{\"message\":\"Error at loading structure. %s\"}}", e.message());
        }

        out.writeChar(0);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoJson(int item)
{
    INDIGO_BEGIN
    {
        auto& tmp = self.getThreadTmpData();
        ArrayOutput out(tmp.string);

        IndigoObject& obj = self.getObject(item);

        if (IndigoBaseMolecule::is(obj))
        {
            MoleculeJsonSaver jn(out);
            self.initMoleculeJsonSaver(jn);
            BaseMolecule& bmol = obj.getBaseMolecule();
            jn.saveMolecule(bmol);
        }
        else if (IndigoBaseReaction::is(obj))
        {
            ReactionJsonSaver jn(out);
            self.initReactionJsonSaver(jn);
            BaseReaction& br = obj.getBaseReaction();
            jn.saveReaction(br);
        }
        out.writeChar(0);
        return tmp.string.ptr();
    }
    INDIGO_END(0);
}

CEXPORT const char* indigoGetOriginalFormat(int item)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(item);
        int original_format = BaseMolecule::UNKNOWN;
        if (IndigoBaseMolecule::is(obj))
        {
            BaseMolecule& mol = obj.getBaseMolecule();
            original_format = mol.original_format;
        }
        else if (IndigoBaseReaction::is(obj))
        {
            BaseReaction& rxn = obj.getBaseReaction();
            original_format = rxn.original_format;
        }
        else
            throw IndigoError("indigoSaveJson(): expected molecule, got %s", obj.debugInfo());

        switch (original_format)
        {
        case BaseMolecule::CML:
            return "chemical/x-cml";
        case BaseMolecule::CDXML:
            return "chemical/x-cdxml";
        case BaseMolecule::CDX:
            return "chemical/x-cdx";
        case BaseMolecule::RDF:
            return "chemical/x-mdl-rdfile";
        case BaseMolecule::SMILES:
            return "chemical/x-daylight-smiles";
        case BaseMolecule::CXSMILES:
            return "chemical/x-chemaxon-cxsmiles";
        case BaseMolecule::SMARTS:
            return "chemical/x-daylight-smarts";
        case BaseMolecule::MOL:
            return "chemical/x-mdl-molfile";
        case BaseMolecule::RXN:
            return "chemical/x-mdl-rxnfile";
        case BaseMolecule::KET:
            return "chemical/x-indigo-ket";
        default:
            return "unknown";
        }
    }
    INDIGO_END(0);
}
